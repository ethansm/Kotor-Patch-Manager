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
    /// Resolves the install set for a request that may involve providers.
    /// </summary>
    /// <remarks>
    /// Providers are derived, never selected. The set is the requested non-provider patches plus
    /// exactly the providers they reach through <see cref="PatchManifest.Requires"/>, transitively.
    /// A requested provider nothing in the set needs is dropped and reported. Every provider installs
    /// before every non-provider patch, ordered among themselves by dependency; the non-provider
    /// patches keep the order <see cref="CalculateInstallOrder"/> gives them, so a request with no
    /// provider in it resolves to exactly what that method returns.
    /// </remarks>
    /// <param name="available">Every patch that could be installed, by ID</param>
    /// <param name="requested">Patch IDs the user asked for, in the order they arranged them</param>
    /// <returns>The final install order with what was auto-added and skipped, or the reason it cannot be resolved</returns>
    public static PatchResult<ProviderResolution> ResolveWithProviders(
        Dictionary<string, PatchManifest> available,
        IEnumerable<string> requested)
    {
        var requestedIds = requested.Distinct().ToList();
        var errors = new List<string>();

        var requestedProviders = new List<string>();
        var nonProviders = new List<string>();
        foreach (var id in requestedIds)
        {
            if (!available.TryGetValue(id, out var manifest))
            {
                errors.Add($"Patch '{id}' not found");
            }
            else if (manifest.IsProvider)
            {
                requestedProviders.Add(id);
            }
            else
            {
                nonProviders.Add(id);
            }
        }

        var nonProviderSet = nonProviders.ToHashSet();

        // Providers reached from the non-provider patches, in discovery order, with who asked for each.
        var reached = new List<string>();
        var requiredBy = new Dictionary<string, List<string>>();

        void Reach(string provider, string by)
        {
            if (!requiredBy.TryGetValue(provider, out var askers))
            {
                requiredBy[provider] = askers = new List<string>();
                reached.Add(provider);
            }

            if (!askers.Contains(by))
            {
                askers.Add(by);
            }
        }

        foreach (var id in nonProviders)
        {
            var missing = new List<string>();
            foreach (var req in available[id].Requires)
            {
                if (!available.TryGetValue(req, out var reqManifest))
                {
                    missing.Add($"{req} (not available)");
                }
                else if (reqManifest.IsProvider)
                {
                    Reach(req, id);
                }
                else if (!nonProviderSet.Contains(req))
                {
                    missing.Add(req);
                }
            }

            if (missing.Count > 0)
            {
                errors.Add($"Patch '{id}' requires: {string.Join(", ", missing)}");
            }
        }

        // Follow provider-to-provider requirements until nothing new turns up. The list grows as it is walked.
        for (var i = 0; i < reached.Count; i++)
        {
            var provider = reached[i];
            foreach (var req in available[provider].Requires)
            {
                if (!available.TryGetValue(req, out var reqManifest))
                {
                    errors.Add($"Provider '{provider}' requires '{req}', which is not available");
                }
                else if (!reqManifest.IsProvider)
                {
                    errors.Add($"Provider '{provider}' requires '{req}', which is not a provider");
                }
                else
                {
                    Reach(req, provider);
                }
            }
        }

        if (errors.Count > 0)
        {
            return PatchResult<ProviderResolution>.Fail(
                $"Dependency validation failed:\n  - {string.Join("\n  - ", errors)}"
            );
        }

        // Providers among themselves: dependencies first, otherwise in discovery order.
        var providerOrder = new List<string>();
        var done = new HashSet<string>();
        var path = new List<string>();

        string? Visit(string provider)
        {
            if (done.Contains(provider))
            {
                return null;
            }

            if (path.Contains(provider))
            {
                var cycle = path.SkipWhile(p => p != provider).Append(provider);
                return $"Circular dependency among providers: {string.Join(" -> ", cycle)}";
            }

            path.Add(provider);
            foreach (var req in available[provider].Requires)
            {
                var problem = Visit(req);
                if (problem != null)
                {
                    return problem;
                }
            }

            path.RemoveAt(path.Count - 1);
            done.Add(provider);
            providerOrder.Add(provider);
            return null;
        }

        foreach (var provider in reached)
        {
            var problem = Visit(provider);
            if (problem != null)
            {
                return PatchResult<ProviderResolution>.Fail(problem);
            }
        }

        var orderResult = CalculateInstallOrder(available, nonProviders);
        if (!orderResult.Success || orderResult.Data == null)
        {
            return PatchResult<ProviderResolution>.Fail(orderResult.Error ?? "Failed to calculate install order");
        }

        var requestedProviderSet = requestedProviders.ToHashSet();
        var autoAdded = providerOrder
            .Where(id => !requestedProviderSet.Contains(id))
            .Select(id => new ProviderAutoAdd(id, requiredBy[id]))
            .ToList();
        var skipped = requestedProviders.Where(id => !requiredBy.ContainsKey(id)).ToList();

        var messages = new List<string>();
        messages.AddRange(autoAdded.Select(a =>
            $"Auto-added provider '{a.ProviderId}' (required by {string.Join(", ", a.RequiredBy)})"));
        messages.AddRange(skipped.Select(id =>
            $"Skipped provider '{id}': no installed mod requires it"));

        var resolution = new ProviderResolution(
            providerOrder.Concat(orderResult.Data).ToList(),
            autoAdded,
            skipped);

        var result = PatchResult<ProviderResolution>.Ok(resolution);
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
/// A provider that was added to an install because other patches need it.
/// </summary>
/// <param name="ProviderId">The provider's patch ID</param>
/// <param name="RequiredBy">IDs of the patches or providers that require it directly</param>
public sealed record ProviderAutoAdd(string ProviderId, IReadOnlyList<string> RequiredBy);

/// <summary>
/// What <see cref="DependencyValidator.ResolveWithProviders"/> decided.
/// </summary>
/// <param name="InstallOrder">Final install set in install order: providers first, then patches</param>
/// <param name="AutoAdded">Providers added because something requires them, in install order</param>
/// <param name="SkippedProviders">Requested providers dropped because nothing in the set requires them</param>
public sealed record ProviderResolution(
    IReadOnlyList<string> InstallOrder,
    IReadOnlyList<ProviderAutoAdd> AutoAdded,
    IReadOnlyList<string> SkippedProviders);
