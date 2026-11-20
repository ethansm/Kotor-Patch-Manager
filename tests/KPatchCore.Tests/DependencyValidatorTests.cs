using Xunit;

using KPatchCore.Models;
using KPatchCore.Validators;

namespace KPatchCore.Tests;

/// <summary>
/// Install-set resolution for providers: derived from what needs them, installed first.
/// </summary>
public class DependencyValidatorTests
{
    private static PatchManifest Make(string id, bool provider = false, params string[] requires) => new()
    {
        Id = id,
        Name = id,
        Version = "1.0.0",
        Author = "Tester",
        Description = id,
        Kind = provider ? PatchManifest.KindProvider : PatchManifest.KindPatch,
        Requires = requires.ToList(),
    };

    private static Dictionary<string, PatchManifest> Catalog(params PatchManifest[] manifests) =>
        manifests.ToDictionary(m => m.Id);

    [Fact]
    public void RequiredProviderIsAutoAdded()
    {
        var catalog = Catalog(Make("lib", true), Make("hud", false, "lib"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "hud" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "lib", "hud" }, result.Data!.InstallOrder);
        var added = Assert.Single(result.Data.AutoAdded);
        Assert.Equal("lib", added.ProviderId);
        Assert.Equal(new[] { "hud" }, added.RequiredBy);
        Assert.Empty(result.Data.SkippedProviders);
        Assert.Contains(result.Messages, m => m.Contains("Auto-added provider 'lib'") && m.Contains("hud"));
    }

    [Fact]
    public void RequiredByListsEveryDirectRequirer()
    {
        var catalog = Catalog(Make("lib", true), Make("hud", false, "lib"), Make("bar", false, "lib"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "hud", "bar" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "hud", "bar" }, Assert.Single(result.Data!.AutoAdded).RequiredBy);
    }

    [Fact]
    public void ProviderRequiringProviderIsAddedTransitivelyAndOrderedByDependency()
    {
        var catalog = Catalog(
            Make("base", true),
            Make("mid", true, "base"),
            Make("hud", false, "mid"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "hud" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "base", "mid", "hud" }, result.Data!.InstallOrder);
        Assert.Equal(new[] { "base", "mid" }, result.Data.AutoAdded.Select(a => a.ProviderId));
        Assert.Equal(new[] { "mid" }, result.Data.AutoAdded[0].RequiredBy);
    }

    [Fact]
    public void ProvidersInstallBeforeEveryNonProviderPatch()
    {
        var catalog = Catalog(
            Make("plain"),
            Make("lib", true),
            Make("hud", false, "lib"),
            Make("other"));

        // The user's own order puts the provider consumer in the middle.
        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "plain", "hud", "other" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "lib", "plain", "hud", "other" }, result.Data!.InstallOrder);
    }

    [Fact]
    public void RequestedProviderNobodyNeedsIsDroppedAndReported()
    {
        var catalog = Catalog(Make("lib", true), Make("plain"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "lib", "plain" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "plain" }, result.Data!.InstallOrder);
        Assert.Equal(new[] { "lib" }, result.Data.SkippedProviders);
        Assert.Empty(result.Data.AutoAdded);
        Assert.Contains(result.Messages, m => m == "Skipped provider 'lib': no installed mod requires it");
    }

    [Fact]
    public void RequestedProviderThatIsNeededIsKeptAndNotReportedAsAutoAdded()
    {
        var catalog = Catalog(Make("lib", true), Make("hud", false, "lib"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "lib", "hud" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "lib", "hud" }, result.Data!.InstallOrder);
        Assert.Empty(result.Data.AutoAdded);
        Assert.Empty(result.Data.SkippedProviders);
    }

    [Fact]
    public void ProviderNeededOnlyByAnotherUnneededProviderIsDropped()
    {
        var catalog = Catalog(Make("base", true), Make("mid", true, "base"), Make("plain"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "mid", "base", "plain" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "plain" }, result.Data!.InstallOrder);
        Assert.Equal(new[] { "mid", "base" }, result.Data.SkippedProviders);
    }

    [Fact]
    public void MissingNonProviderRequirementFails()
    {
        var catalog = Catalog(Make("dep"), Make("child", false, "dep"));

        // dep is available but was not requested, which fails exactly as ValidateDependencies does.
        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "child" });

        Assert.False(result.Success);
        Assert.Contains("Patch 'child' requires: dep", result.Error);
    }

    [Fact]
    public void RequirementNotAvailableAtAllFails()
    {
        var catalog = Catalog(Make("child", false, "ghost"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "child" });

        Assert.False(result.Success);
        Assert.Contains("ghost", result.Error);
    }

    [Fact]
    public void MissingProviderRequirementFails()
    {
        var catalog = Catalog(Make("hud", false, "no-such-lib"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "hud" });

        Assert.False(result.Success);
        Assert.Contains("no-such-lib (not available)", result.Error);
    }

    [Fact]
    public void ProviderWithMissingProviderRequirementFails()
    {
        var catalog = Catalog(Make("lib", true, "gone"), Make("hud", false, "lib"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "hud" });

        Assert.False(result.Success);
        Assert.Contains("Provider 'lib' requires 'gone'", result.Error);
    }

    [Fact]
    public void ProviderRequiringNonProviderFails()
    {
        var catalog = Catalog(Make("plain"), Make("lib", true, "plain"), Make("hud", false, "lib"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "hud", "plain" });

        Assert.False(result.Success);
        Assert.Contains("not a provider", result.Error);
    }

    [Fact]
    public void UnknownRequestedPatchFails()
    {
        var result = DependencyValidator.ResolveWithProviders(Catalog(Make("a")), new[] { "nope" });

        Assert.False(result.Success);
        Assert.Contains("'nope' not found", result.Error);
    }

    [Fact]
    public void ProviderCycleIsDetected()
    {
        var catalog = Catalog(
            Make("a", true, "b"),
            Make("b", true, "a"),
            Make("hud", false, "a"));

        var result = DependencyValidator.ResolveWithProviders(catalog, new[] { "hud" });

        Assert.False(result.Success);
        Assert.Contains("Circular dependency among providers", result.Error);
        Assert.Contains("a -> b -> a", result.Error);
    }

    [Fact]
    public void ProviderFreeOrderMatchesCalculateInstallOrder()
    {
        var catalog = Catalog(
            Make("c"),
            Make("b", false, "a"),
            Make("a"),
            Make("d", false, "c", "b"),
            Make("e"));
        var request = new[] { "e", "d", "b", "c", "a" };

        var old = DependencyValidator.CalculateInstallOrder(catalog, request);
        var result = DependencyValidator.ResolveWithProviders(catalog, request);

        Assert.True(old.Success);
        Assert.True(result.Success, result.Error);
        Assert.Equal(old.Data, result.Data!.InstallOrder);
        Assert.Empty(result.Data.AutoAdded);
        Assert.Empty(result.Data.SkippedProviders);
        Assert.Empty(result.Messages);
    }

    [Fact]
    public void NonProviderOrderIsUnchangedByProvidersBeingPresent()
    {
        var catalog = Catalog(
            Make("lib", true),
            Make("child", false, "parent", "lib"),
            Make("parent"),
            Make("solo"));
        var request = new[] { "solo", "child", "parent" };

        var old = DependencyValidator.CalculateInstallOrder(
            Catalog(Make("child", false, "parent"), Make("parent"), Make("solo")), request);
        var result = DependencyValidator.ResolveWithProviders(catalog, request);

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "lib" }.Concat(old.Data!), result.Data!.InstallOrder);
    }

    // FM1a regression: PatchApplicator re-sorts the resolved install set with CalculateInstallOrder.
    // That must keep providers first and list nothing twice.
    [Fact]
    public void ResortingTheResolvedSetIsStableAndHasNoDuplicates()
    {
        var catalog = Catalog(
            Make("base", true),
            Make("mid", true, "base"),
            Make("hud", false, "mid", "base"),
            Make("bar", false, "mid"));

        var resolved = DependencyValidator.ResolveWithProviders(catalog, new[] { "hud", "bar" });
        Assert.True(resolved.Success, resolved.Error);

        var resorted = DependencyValidator.CalculateInstallOrder(catalog, resolved.Data!.InstallOrder);
        Assert.True(resorted.Success, resorted.Error);
        Assert.Equal(resolved.Data.InstallOrder, resorted.Data);
        Assert.Equal(resorted.Data!.Count, resorted.Data.Distinct().Count());
    }

    // FM1a regression: option handling in PatchApplicator now walks the resolved install set, not the
    // requested IDs. An auto-added provider is in that set and not in the request; a requested provider
    // nothing needs is in the request and not in the set (indexing the set's entries by it threw
    // KeyNotFoundException).
    [Fact]
    public void AutoAddedProviderIsInTheInstallSetButNotTheRequest_AndSkippedOneIsNot()
    {
        var catalog = Catalog(Make("lib", true), Make("idle", true), Make("hud", false, "lib"));
        var requested = new[] { "hud", "idle" };

        var result = DependencyValidator.ResolveWithProviders(catalog, requested);

        Assert.True(result.Success, result.Error);
        Assert.Contains("lib", result.Data!.InstallOrder);
        Assert.DoesNotContain("lib", requested);
        Assert.DoesNotContain("idle", result.Data.InstallOrder);
        Assert.Equal(new[] { "idle" }, result.Data.SkippedProviders);
    }

    // requires_interfaces is informational by design (PatchManifest doc; local-mods adds no later
    // enforcement): only Requires drives resolution.
    [Fact]
    public void RequiresInterfacesDoesNotAffectResolution()
    {
        var hud = Make("hud");
        hud = new PatchManifest
        {
            Id = hud.Id, Name = hud.Name, Version = hud.Version, Author = hud.Author,
            Description = hud.Description, Kind = hud.Kind,
            RequiresInterfaces = new List<string> { "k2.missing@1" },
        };

        var result = DependencyValidator.ResolveWithProviders(Catalog(hud), new[] { "hud" });

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "hud" }, result.Data!.InstallOrder);
    }
}
