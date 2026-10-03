using Xunit;

using KPatchCore.Parsers;

namespace KPatchCore.Tests;

/// <summary>
/// The manifest fields that say what kind of patch this is and what interfaces it trades in.
/// </summary>
public class ManifestParserTests
{
    private const string Required = """
        id = "sample"
        name = "Sample"
        version = "1.0.0"
        author = "Tester"
        description = "A sample patch"
        """;

    private static string Manifest(string extra = "") => $"[patch]\n{Required}\n{extra}\n";

    public static TheoryData<string> ShippedManifests()
    {
        var data = new TheoryData<string>();
        foreach (var path in Directory.EnumerateFiles(
                     RepositoryFiles.Patches, "manifest.toml", SearchOption.AllDirectories)
                     .OrderBy(p => p, StringComparer.Ordinal))
        {
            data.Add(Path.GetRelativePath(RepositoryFiles.Patches, path));
        }

        return data;
    }

    [Fact]
    public void OldManifestParsesWithPatchKindAndNoInterfaces()
    {
        var result = ManifestParser.ParseString(Manifest());

        Assert.True(result.Success, result.Error);
        Assert.Equal("patch", result.Data!.Kind);
        Assert.False(result.Data.IsLibrary);
        Assert.Empty(result.Data.Provides);
        Assert.Empty(result.Data.RequiresInterfaces);
    }

    [Fact]
    public void LibraryKindIsRecognised()
    {
        var result = ManifestParser.ParseString(Manifest("kind = \"library\""));

        Assert.True(result.Success, result.Error);
        Assert.Equal("library", result.Data!.Kind);
        Assert.True(result.Data.IsLibrary);
    }

    [Fact]
    public void ExplicitPatchKindIsRecognised()
    {
        var result = ManifestParser.ParseString(Manifest("kind = \"patch\""));

        Assert.True(result.Success, result.Error);
        Assert.False(result.Data!.IsLibrary);
    }

    [Theory]
    [InlineData("kind = \"plugin\"")]
    [InlineData("kind = \"Library\"")]
    [InlineData("kind = \"\"")]
    [InlineData("kind = 3")]
    public void UnknownKindIsAParseError(string line)
    {
        var result = ManifestParser.ParseString(Manifest(line));

        Assert.False(result.Success);
        Assert.Contains("patch.kind", result.Error);
    }

    [Fact]
    public void ValidInterfacesAreRead()
    {
        var result = ManifestParser.ParseString(Manifest("""
            provides = ["k2.portraitslots@1", "other_iface-2@12"]
            requires_interfaces = ["k2.portraitslots@1"]
            """));

        Assert.True(result.Success, result.Error);
        Assert.Equal(new[] { "k2.portraitslots@1", "other_iface-2@12" }, result.Data!.Provides);
        Assert.Equal(new[] { "k2.portraitslots@1" }, result.Data.RequiresInterfaces);
    }

    [Theory]
    [InlineData("provides = [\"k2.portraitslots\"]")]
    [InlineData("provides = [\"k2.portraitslots@\"]")]
    [InlineData("provides = [\"K2.PortraitSlots@1\"]")]
    [InlineData("provides = [\"k2.portraitslots@1.2\"]")]
    [InlineData("provides = [\"k2 portraitslots@1\"]")]
    [InlineData("provides = [\"@1\"]")]
    [InlineData("provides = [3]")]
    [InlineData("provides = \"k2.portraitslots@1\"")]
    [InlineData("requires_interfaces = [\"nover\"]")]
    [InlineData("requires_interfaces = [\"ok@1\", \"bad@x\"]")]
    public void InvalidInterfaceIsAParseError(string line)
    {
        var result = ManifestParser.ParseString(Manifest(line));

        Assert.False(result.Success);
        Assert.NotNull(result.Error);
    }

    [Theory]
    [MemberData(nameof(ShippedManifests))]
    public void ShippedManifestParses(string relativePath)
    {
        var result = ManifestParser.ParseFile(Path.Combine(RepositoryFiles.Patches, relativePath));

        Assert.True(result.Success, $"{relativePath}: {result.Error}");
    }

    [Fact]
    public void ShippedManifestsExist()
    {
        Assert.NotEmpty(ShippedManifests());
    }
}
