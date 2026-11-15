#:package Tomlyn@2.11.1

using Tomlyn;
using Tomlyn.Model;
using Tomlyn.Serialization;
using System.Runtime.CompilerServices;

Console.ForegroundColor = ConsoleColor.White;

static string GetScriptPath([CallerFilePath] string path = "") => path;
string scriptDirectory = Path.GetDirectoryName(GetScriptPath())!;
string originalFilePath = Path.Combine(scriptDirectory, "Protocol.toml");

if (!File.Exists(originalFilePath))
    throw new FileNotFoundException("The file was not found!");

string tomlStr = File.ReadAllText(originalFilePath);
TomlTable root =
    TomlSerializer.Deserialize(tomlStr, MyTomlContext.Default.TomlTable) ?? throw new ArgumentException("Toml couldn't be deserialized!");;

TomlTableArray headerRoot = (TomlTableArray)root["header"];
TomlTable tomlHeaders =
    headerRoot[0] ?? throw new ArgumentException("Header section was not found!");

long version = (long)tomlHeaders["version"]!;
long maxPayloadSize = (long)tomlHeaders["maxPayloadSize"]!;

TomlTableArray tomlMessages = (TomlTableArray)root["message"];
Message[] messages = new Message[tomlMessages.Count];

Console.WriteLine($"Version: {version}");
Console.WriteLine($"Max Payload: {maxPayloadSize}");

foreach (var (i, msg) in tomlMessages.Index())
{
    string msgName = (string)msg["name"];
    if (String.IsNullOrWhiteSpace(msgName))
        throw new ArgumentException("The message name needs to be non-null, non-empty, and not-only-whitespaces");

    UInt16 id = checked((UInt16)i);

    var tomlFields = (TomlArray)msg["fields"];

    Field[] fields = new Field[tomlFields.Count];

    foreach (var (j, field) in tomlFields.Index())
    {
        var fieldTyped = (TomlTable)field!;

        EType type = ConvertTypeToEnum((string)fieldTyped["type"]);
        
        string fieldName = (string)fieldTyped["name"];
        if (String.IsNullOrWhiteSpace(fieldName))
            throw new ArgumentException("The field name  needs to be non-null, non-empty, and not-only-whitespaces");

        fields[j] = new Field
        {
            Type = type,
            Name = fieldName
        };
    }

    messages[i] = new Message
    {
        Name = msgName,
        Id = id,
        Fields = fields
    };
}

// control
foreach (var msg in messages)
{
    Console.WriteLine($"Name: {msg.Name}");
    Console.WriteLine($"Id: {msg.Id}");

    if (msg.Fields.Length == 0)
    {
        Console.ForegroundColor = ConsoleColor.Cyan;
        Console.WriteLine($"\tNONE");
        Console.ForegroundColor = ConsoleColor.White;
        continue;
    }
    foreach (var field in msg.Fields)
    {
        Console.WriteLine($"\t{field.Type} {field.Name}");
    }
}

static EType ConvertTypeToEnum(string type) => type switch
{
    "bool"   => EType.Bool,
    "u8"     => EType.Uint8,
    "u16"    => EType.Uint16,
    "u32"    => EType.Uint32,
    "u64"    => EType.Uint64,
    "i8"     => EType.Int8,
    "i16"    => EType.Int16,
    "i32"    => EType.Int32,
    "i64"    => EType.Int64,
    "string" => EType.String,
    _ => throw new InvalidOperationException(
        $"Unsupported protocol field type: '{type}'")
};

// Console.WriteLine(tomlStr);
[TomlSerializable(typeof(TomlTable))]
internal partial class MyTomlContext : TomlSerializerContext { }

class Field
{
    public required EType Type;
    public required string Name;
};

class Message
{
    public required string Name;
    public required UInt16 Id;
    public required Field[] Fields; // this can be empty though 
};

enum EType : byte
{
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    Uint8,
    Uint16,
    Uint32,
    Uint64,
    String
};
