namespace KPatchCore.Models;

/// <summary>
/// Represents the metadata from a patch's manifest.toml file
/// </summary>
public sealed class PatchManifest
{
    /// <summary>
    /// Unique identifier for the patch (e.g., "widescreen-fix")
    /// </summary>
    public required string Id { get; init; }

    /// <summary>
    /// Human-readable patch name (e.g., "Widescreen Resolution Fix")
    /// </summary>
    public required string Name { get; init; }

    /// <summary>
    /// Semantic version (e.g., "1.2.0")
    /// </summary>
    public required string Version { get; init; }

    /// <summary>
    /// Patch author/creator
    /// </summary>
    public required string Author { get; init; }

    /// <summary>
    /// Description of what the patch does
    /// </summary>
    public required string Description { get; init; }

    /// <summary>
    /// List of patch IDs that must be installed before this one
    /// </summary>
    public List<string> Requires { get; init; } = new();

    /// <summary>
    /// List of patch IDs that conflict with this patch
    /// </summary>
    public List<string> Conflicts { get; init; } = new();

    /// <summary>
    /// What this manifest describes: <see cref="KindPatch"/> (the default) or
    /// <see cref="KindProvider"/>. A provider owns
    /// shared engine hooks and provides an interface to other mods; it is derived from what
    /// is installed and never selected directly.
    /// </summary>
    public string Kind { get; init; } = KindPatch;

    /// <summary>
    /// True when <see cref="Kind"/> is <see cref="KindProvider"/>
    /// </summary>
    public bool IsProvider => Kind == KindProvider;

    /// <summary>
    /// Interfaces this patch provides, as <c>name@major</c> (e.g., "k2.portraitslots@1")
    /// </summary>
    public List<string> Provides { get; init; } = new();

    /// <summary>
    /// Interfaces this patch consumes, as <c>name@major</c>. Informational: dependencies
    /// are resolved through <see cref="Requires"/>.
    /// </summary>
    public List<string> RequiresInterfaces { get; init; } = new();

    /// <summary>The ordinary kind of patch</summary>
    public const string KindPatch = "patch";

    /// <summary>The shared service provider kind of patch</summary>
    public const string KindProvider = "provider";

    /// <summary>
    /// Game versions this patch supports (key = GameVersion.Id, value = expected hash)
    /// </summary>
    public Dictionary<string, string> SupportedVersions { get; init; } = new();

    /// <summary>
    /// Choices the player can make inside this patch ([[patch.options]]), in display order
    /// </summary>
    public List<PatchOption> Options { get; init; } = new();

    /// <summary>
    /// Optional URL to patch homepage/documentation
    /// </summary>
    public string? Url { get; init; }

    /// <summary>
    /// Optional license information
    /// </summary>
    public string? License { get; init; }

    public override string ToString() =>
        $"{Name} v{Version} by {Author}";
}
