using Xunit;

using KPatchCore.Models;
using KPatchCore.Validators;

namespace KPatchCore.Tests;

/// <summary>
/// Install-set resolution for libraries: derived from what needs them, installed first.
/// </summary>
public class DependencyValidatorTests
{
    private static PatchManifest Make(string id, bool library = false, params string[] requires) => new()
    {
        Id = id,
        Name = id,
        Version = "1.0.0",
        Author = "Tester",
        Description = id,
        Kind = library ? PatchManifest.KindLibrary : PatchManifest.KindPatch,
        Requires = requires.ToList(),
    };

    private static Dictionary<string, PatchManifest> Catalog(params PatchManifest[] manifests) =>
        manifests.ToDictionary(m => m.Id);

    [Fact]
    public void RequiredLibraryIsAutoAdded()
    {
        var catalog = Catalog(Make("lib", true), Make("hud", false, "lib"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "hud" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "lib", "hud" }, result.Data!.InstallOrder);
        var added = Assert.Single(result.Data.AutoAdded);
        Assert.Equal("lib", added.LibraryId);
        Assert.Equal(new[] { "hud" }, added.RequiredBy);
        Assert.Empty(result.Data.SkippedLibraries);
        Assert.Contains(result.Messages, m => m.Contains("Auto-added library 'lib'") && m.Contains("hud"));
    }

    [Fact]
    public void RequiredByListsEveryDirectRequirer()
    {
        var catalog = Catalog(Make("lib", true), Make("hud", false, "lib"), Make("bar", false, "lib"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "hud", "bar" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "hud", "bar" }, Assert.Single(result.Data!.AutoAdded).RequiredBy);
    }

    [Fact]
    public void LibraryRequiringLibraryIsAddedTransitivelyAndOrderedByDependency()
    {
        var catalog = Catalog(
            Make("base", true),
            Make("mid", true, "base"),
            Make("hud", false, "mid"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "hud" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "base", "mid", "hud" }, result.Data!.InstallOrder);
        Assert.Equal(new[] { "base", "mid" }, result.Data.AutoAdded.Select(a => a.LibraryId));
        Assert.Equal(new[] { "mid" }, result.Data.AutoAdded[0].RequiredBy);
    }

    [Fact]
    public void LibrariesInstallBeforeEveryNonLibraryPatch()
    {
        var catalog = Catalog(
            Make("plain"),
            Make("lib", true),
            Make("hud", false, "lib"),
            Make("other"));

        // The user's own order puts the library consumer in the middle.
        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "plain", "hud", "other" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "lib", "plain", "hud", "other" }, result.Data!.InstallOrder);
    }

    [Fact]
    public void RequestedLibraryNobodyNeedsIsDroppedAndReported()
    {
        var catalog = Catalog(Make("lib", true), Make("plain"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "lib", "plain" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "plain" }, result.Data!.InstallOrder);
        Assert.Equal(new[] { "lib" }, result.Data.SkippedLibraries);
        Assert.Empty(result.Data.AutoAdded);
        Assert.Contains(result.Messages, m => m == "Skipped library 'lib': no installed mod requires it");
    }

    [Fact]
    public void RequestedLibraryThatIsNeededIsKeptAndNotReportedAsAutoAdded()
    {
        var catalog = Catalog(Make("lib", true), Make("hud", false, "lib"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "lib", "hud" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "lib", "hud" }, result.Data!.InstallOrder);
        Assert.Empty(result.Data.AutoAdded);
        Assert.Empty(result.Data.SkippedLibraries);
    }

    [Fact]
    public void LibraryNeededOnlyByAnotherUnneededLibraryIsDropped()
    {
        var catalog = Catalog(Make("base", true), Make("mid", true, "base"), Make("plain"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "mid", "base", "plain" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "plain" }, result.Data!.InstallOrder);
        Assert.Equal(new[] { "mid", "base" }, result.Data.SkippedLibraries);
    }

    [Fact]
    public void MissingNonLibraryRequirementFails()
    {
        var catalog = Catalog(Make("dep"), Make("child", false, "dep"));

        // dep is available but was not requested, which fails exactly as ValidateDependencies does.
        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "child" });

        Assert.False(result.Success);
        Assert.Contains("Patch 'child' requires: dep", result.Error);
    }

    [Fact]
    public void RequirementNotAvailableAtAllFails()
    {
        var catalog = Catalog(Make("child", false, "ghost"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "child" });

        Assert.False(result.Success);
        Assert.Contains("ghost", result.Error);
    }

    [Fact]
    public void MissingLibraryRequirementFails()
    {
        var catalog = Catalog(Make("hud", false, "no-such-lib"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "hud" });

        Assert.False(result.Success);
        Assert.Contains("no-such-lib (not available)", result.Error);
    }

    [Fact]
    public void LibraryWithMissingLibraryRequirementFails()
    {
        var catalog = Catalog(Make("lib", true, "gone"), Make("hud", false, "lib"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "hud" });

        Assert.False(result.Success);
        Assert.Contains("Library 'lib' requires 'gone'", result.Error);
    }

    [Fact]
    public void LibraryRequiringNonLibraryFails()
    {
        var catalog = Catalog(Make("plain"), Make("lib", true, "plain"), Make("hud", false, "lib"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "hud", "plain" });

        Assert.False(result.Success);
        Assert.Contains("not a library", result.Error);
    }

    [Fact]
    public void UnknownRequestedPatchFails()
    {
        var result = DependencyValidator.ResolveWithLibraries(Catalog(Make("a")), new[] { "nope" });

        Assert.False(result.Success);
        Assert.Contains("'nope' not found", result.Error);
    }

    [Fact]
    public void LibraryCycleIsDetected()
    {
        var catalog = Catalog(
            Make("a", true, "b"),
            Make("b", true, "a"),
            Make("hud", false, "a"));

        var result = DependencyValidator.ResolveWithLibraries(catalog, new[] { "hud" });

        Assert.False(result.Success);
        Assert.Contains("Circular dependency among libraries", result.Error);
        Assert.Contains("a -> b -> a", result.Error);
    }

    [Fact]
    public void LibraryFreeOrderMatchesCalculateInstallOrder()
    {
        var catalog = Catalog(
            Make("c"),
            Make("b", false, "a"),
            Make("a"),
            Make("d", false, "c", "b"),
            Make("e"));
        var request = new[] { "e", "d", "b", "c", "a" };

        var old = DependencyValidator.CalculateInstallOrder(catalog, request);
        var result = DependencyValidator.ResolveWithLibraries(catalog, request);

        Assert.True(old.Success);
        Assert.True(result.Success, result.Error);
        Assert.Equal(old.Data, result.Data!.InstallOrder);
        Assert.Empty(result.Data.AutoAdded);
        Assert.Empty(result.Data.SkippedLibraries);
        Assert.Empty(result.Messages);
    }

    [Fact]
    public void NonLibraryOrderIsUnchangedByLibrariesBeingPresent()
    {
        var catalog = Catalog(
            Make("lib", true),
            Make("child", false, "parent", "lib"),
            Make("parent"),
            Make("solo"));
        var request = new[] { "solo", "child", "parent" };

        var old = DependencyValidator.CalculateInstallOrder(
            Catalog(Make("child", false, "parent"), Make("parent"), Make("solo")), request);
        var result = DependencyValidator.ResolveWithLibraries(catalog, request);

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "lib" }.Concat(old.Data!), result.Data!.InstallOrder);
    }
}
