using KPatchCore.Models;

namespace KPatchCore.Validators;

/// <summary>
/// Validates patch dependencies and conflicts
/// </summary>
public static class DependencyValidator
{
    /// <summary>
    /// Validates that all dependencies are satisfied for a set of patches
    /// </summary>
    /// <param name="patches">Dictionary of patch ID to manifest</param>
    /// <param name="patchesToInstall">List of patch IDs to install</param>
    /// <returns>Result indicating if all dependencies are satisfied</returns>
    public static PatchResult ValidateDependencies(
        Dictionary<string, PatchManifest> patches,
        IEnumerable<string> patchesToInstall)
    {
        var toInstall = patchesToInstall.ToHashSet();
        var errors = new List<string>();

        foreach (var patchId in toInstall)
        {
            if (!patches.TryGetValue(patchId, out var manifest))
            {
                errors.Add($"Patch '{patchId}' not found");
                continue;
            }

            // Check if all required patches are either being installed or already available
            var missing = manifest.Requires.Where(req => !toInstall.Contains(req)).ToList();
            if (missing.Count > 0)
            {
                errors.Add(
                    $"Patch '{patchId}' requires: {string.Join(", ", missing)}"
                );
            }
        }

        if (errors.Count > 0)
        {
            return PatchResult.Fail(
                $"Dependency validation failed:\n  - {string.Join("\n  - ", errors)}"
            );
        }

        return PatchResult.Ok("All dependencies satisfied");
    }

    /// <summary>
    /// Detects circular dependencies in patch requirements
    /// </summary>
    /// <param name="patches">Dictionary of patch ID to manifest</param>
    /// <returns>Result indicating if there are circular dependencies</returns>
    public static PatchResult DetectCircularDependencies(Dictionary<string, PatchManifest> patches)
    {
        var cycles = new List<string>();

        foreach (var patchId in patches.Keys)
        {
            var visited = new HashSet<string>();
            var path = new List<string>();

            if (HasCircularDependency(patchId, patches, visited, path))
            {
                cycles.Add($"Circular dependency: {string.Join(" -> ", path)} -> {patchId}");
            }
        }

        if (cycles.Count > 0)
        {
            return PatchResult.Fail(
                $"Circular dependencies detected:\n  - {string.Join("\n  - ", cycles)}"
            );
        }

        return PatchResult.Ok("No circular dependencies detected");
    }

    /// <summary>
    /// Validates that no conflicts exist between patches being installed
    /// </summary>
    /// <param name="patches">Dictionary of patch ID to manifest</param>
    /// <param name="patchesToInstall">List of patch IDs to install</param>
    /// <returns>Result indicating if there are any conflicts</returns>
    public static PatchResult ValidateNoConflicts(
        Dictionary<string, PatchManifest> patches,
        IEnumerable<string> patchesToInstall)
    {
        var toInstall = patchesToInstall.ToHashSet();
        var errors = new List<string>();

        foreach (var patchId in toInstall)
        {
            if (!patches.TryGetValue(patchId, out var manifest))
            {
                continue;
            }

            // Check if any conflicting patches are in the install list
            var conflicts = manifest.Conflicts.Where(conf => toInstall.Contains(conf)).ToList();
            if (conflicts.Count > 0)
            {
                errors.Add(
                    $"Patch '{patchId}' conflicts with: {string.Join(", ", conflicts)}"
                );
            }
        }

        if (errors.Count > 0)
        {
            return PatchResult.Fail(
                $"Conflict validation failed:\n  - {string.Join("\n  - ", errors)}"
            );
        }

        return PatchResult.Ok("No conflicts detected");
    }

    /// <summary>
    /// Resolves the install set for a request that may involve libraries.
    /// </summary>
    /// <remarks>
    /// Libraries are derived, never selected. The set is the requested non-library patches plus
    /// exactly the libraries they reach through <see cref="PatchManifest.Requires"/>, transitively.
    /// A requested library nothing in the set needs is dropped and reported. Every library installs
    /// before every non-library patch, ordered among themselves by dependency; the non-library
    /// patches keep the order <see cref="CalculateInstallOrder"/> gives them, so a request with no
    /// library in it resolves to exactly what that method returns.
    /// </remarks>
    /// <param name="available">Every patch that could be installed, by ID</param>
    /// <param name="requested">Patch IDs the user asked for, in the order they arranged them</param>
    /// <returns>The final install order with what was auto-added and skipped, or the reason it cannot be resolved</returns>
    public static PatchResult<LibraryResolution> ResolveWithLibraries(
        Dictionary<string, PatchManifest> available,
        IEnumerable<string> requested)
    {
        var requestedIds = requested.Distinct().ToList();
        var errors = new List<string>();

        var requestedLibraries = new List<string>();
        var nonLibraries = new List<string>();
        foreach (var id in requestedIds)
        {
            if (!available.TryGetValue(id, out var manifest))
            {
                errors.Add($"Patch '{id}' not found");
            }
            else if (manifest.IsLibrary)
            {
                requestedLibraries.Add(id);
            }
            else
            {
                nonLibraries.Add(id);
            }
        }

        var nonLibrarySet = nonLibraries.ToHashSet();

        // Libraries reached from the non-library patches, in discovery order, with who asked for each.
        var reached = new List<string>();
        var requiredBy = new Dictionary<string, List<string>>();

        void Reach(string library, string by)
        {
            if (!requiredBy.TryGetValue(library, out var askers))
            {
                requiredBy[library] = askers = new List<string>();
                reached.Add(library);
            }

            if (!askers.Contains(by))
            {
                askers.Add(by);
            }
        }

        foreach (var id in nonLibraries)
        {
            var missing = new List<string>();
            foreach (var req in available[id].Requires)
            {
                if (!available.TryGetValue(req, out var reqManifest))
                {
                    missing.Add($"{req} (not available)");
                }
                else if (reqManifest.IsLibrary)
                {
                    Reach(req, id);
                }
                else if (!nonLibrarySet.Contains(req))
                {
                    missing.Add(req);
                }
            }

            if (missing.Count > 0)
            {
                errors.Add($"Patch '{id}' requires: {string.Join(", ", missing)}");
            }
        }

        // Follow library-to-library requirements until nothing new turns up. The list grows as it is walked.
        for (var i = 0; i < reached.Count; i++)
        {
            var library = reached[i];
            foreach (var req in available[library].Requires)
            {
                if (!available.TryGetValue(req, out var reqManifest))
                {
                    errors.Add($"Library '{library}' requires '{req}', which is not available");
                }
                else if (!reqManifest.IsLibrary)
                {
                    errors.Add($"Library '{library}' requires '{req}', which is not a library");
                }
                else
                {
                    Reach(req, library);
                }
            }
        }

        if (errors.Count > 0)
        {
            return PatchResult<LibraryResolution>.Fail(
                $"Dependency validation failed:\n  - {string.Join("\n  - ", errors)}"
            );
        }

        // Libraries among themselves: dependencies first, otherwise in discovery order.
        var libraryOrder = new List<string>();
        var done = new HashSet<string>();
        var path = new List<string>();

        string? Visit(string library)
        {
            if (done.Contains(library))
            {
                return null;
            }

            if (path.Contains(library))
            {
                var cycle = path.SkipWhile(p => p != library).Append(library);
                return $"Circular dependency among libraries: {string.Join(" -> ", cycle)}";
            }

            path.Add(library);
            foreach (var req in available[library].Requires)
            {
                var problem = Visit(req);
                if (problem != null)
                {
                    return problem;
                }
            }

            path.RemoveAt(path.Count - 1);
            done.Add(library);
            libraryOrder.Add(library);
            return null;
        }

        foreach (var library in reached)
        {
            var problem = Visit(library);
            if (problem != null)
            {
                return PatchResult<LibraryResolution>.Fail(problem);
            }
        }

        var orderResult = CalculateInstallOrder(available, nonLibraries);
        if (!orderResult.Success || orderResult.Data == null)
        {
            return PatchResult<LibraryResolution>.Fail(orderResult.Error ?? "Failed to calculate install order");
        }

        var requestedLibrarySet = requestedLibraries.ToHashSet();
        var autoAdded = libraryOrder
            .Where(id => !requestedLibrarySet.Contains(id))
            .Select(id => new LibraryAutoAdd(id, requiredBy[id]))
            .ToList();
        var skipped = requestedLibraries.Where(id => !requiredBy.ContainsKey(id)).ToList();

        var messages = new List<string>();
        messages.AddRange(autoAdded.Select(a =>
            $"Auto-added library '{a.LibraryId}' (required by {string.Join(", ", a.RequiredBy)})"));
        messages.AddRange(skipped.Select(id =>
            $"Skipped library '{id}': no installed mod requires it"));

        var resolution = new LibraryResolution(
            libraryOrder.Concat(orderResult.Data).ToList(),
            autoAdded,
            skipped);

        var result = PatchResult<LibraryResolution>.Ok(resolution);
        result.Messages.AddRange(messages);
        return result;
    }

    /// <summary>
    /// Calculates the installation order based on dependencies
    /// </summary>
    /// <param name="patches">Dictionary of patch ID to manifest</param>
    /// <param name="patchesToInstall">List of patch IDs to install</param>
    /// <returns>Result containing ordered list of patch IDs or error</returns>
    public static PatchResult<List<string>> CalculateInstallOrder(
        Dictionary<string, PatchManifest> patches,
        IEnumerable<string> patchesToInstall)
    {
        // The caller's sequence is the order the user arranged, and the sort only has to move a
        // patch that something else depends on. Seeding from the set instead would leave the rest
        // in whatever order it happened to enumerate.
        var requested = patchesToInstall.ToList();
        var toInstall = requested.ToHashSet();
        var ordered = new List<string>();
        var visited = new HashSet<string>();

        // Topological sort using DFS
        foreach (var patchId in requested)
        {
            if (!VisitPatch(patchId, patches, toInstall, visited, ordered))
            {
                return PatchResult<List<string>>.Fail(
                    $"Failed to calculate install order - possible circular dependency involving '{patchId}'"
                );
            }
        }

        return PatchResult<List<string>>.Ok(
            ordered,
            $"Install order: {string.Join(" -> ", ordered)}"
        );
    }

    private static bool HasCircularDependency(
        string patchId,
        Dictionary<string, PatchManifest> patches,
        HashSet<string> visited,
        List<string> path)
    {
        if (path.Contains(patchId))
        {
            return true; // Found a cycle
        }

        if (visited.Contains(patchId))
        {
            return false; // Already checked this path
        }

        visited.Add(patchId);
        path.Add(patchId);

        if (patches.TryGetValue(patchId, out var manifest))
        {
            foreach (var dependency in manifest.Requires)
            {
                if (HasCircularDependency(dependency, patches, visited, path))
                {
                    return true;
                }
            }
        }

        path.Remove(patchId);
        return false;
    }

    private static bool VisitPatch(
        string patchId,
        Dictionary<string, PatchManifest> patches,
        HashSet<string> toInstall,
        HashSet<string> visited,
        List<string> ordered)
    {
        if (visited.Contains(patchId))
        {
            return true; // Already processed
        }

        if (!patches.TryGetValue(patchId, out var manifest))
        {
            return false; // Patch not found
        }

        visited.Add(patchId);

        // Visit dependencies first
        foreach (var dependency in manifest.Requires)
        {
            if (toInstall.Contains(dependency))
            {
                if (!VisitPatch(dependency, patches, toInstall, visited, ordered))
                {
                    return false;
                }
            }
        }

        // Add this patch after its dependencies
        ordered.Add(patchId);
        return true;
    }

    /// <summary>
    /// Validates version compatibility between dependent patches
    /// </summary>
    /// <param name="manifest">Patch manifest to check</param>
    /// <param name="installedVersions">Dictionary of installed patch IDs to their versions</param>
    /// <returns>Result indicating if versions are compatible</returns>
    public static PatchResult ValidateVersionCompatibility(
        PatchManifest manifest,
        Dictionary<string, string> installedVersions)
    {
        // For now, this is a simple check that required patches exist
        // In the future, could add version range requirements (e.g., "requires: base-patch >= 1.0")

        var missing = manifest.Requires
            .Where(req => !installedVersions.ContainsKey(req))
            .ToList();

        if (missing.Count > 0)
        {
            return PatchResult.Fail(
                $"Missing required patches: {string.Join(", ", missing)}"
            );
        }

        return PatchResult.Ok("Version compatibility validated");
    }
}

/// <summary>
/// A library that was added to an install because other patches need it.
/// </summary>
/// <param name="LibraryId">The library's patch ID</param>
/// <param name="RequiredBy">IDs of the patches or libraries that require it directly</param>
public sealed record LibraryAutoAdd(string LibraryId, IReadOnlyList<string> RequiredBy);

/// <summary>
/// What <see cref="DependencyValidator.ResolveWithLibraries"/> decided.
/// </summary>
/// <param name="InstallOrder">Final install set in install order: libraries first, then patches</param>
/// <param name="AutoAdded">Libraries added because something requires them, in install order</param>
/// <param name="SkippedLibraries">Requested libraries dropped because nothing in the set requires them</param>
public sealed record LibraryResolution(
    IReadOnlyList<string> InstallOrder,
    IReadOnlyList<LibraryAutoAdd> AutoAdded,
    IReadOnlyList<string> SkippedLibraries);
