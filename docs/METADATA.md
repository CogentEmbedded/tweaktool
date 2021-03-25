# Cogent Tweak Tool Metadata Format

## Uses of Metadata

Metadata field is supplied by the user application when each tweak is created. It is constant and cannot be changed in runtime after that.

Metadata is encoded in utf-8.

Metadata contains information for GUI to build rich user experience.

Metadata can be presented in several formats:

1. JSON
2. printf-friendly 'key=value ' format

GUI auto-detects metadata format and emits a warning to the user if no metadata can be parsed successfully.

Metadata parsing is centered around cheap & easy generation (typically static strings in user application).

## Metadata JSON Format

Typical metadata contains:

```json
{
    // [OPTIONAL] default="slider"
    // Control type, see below.
    "control": "slider",

    // Minimum value for slider or spinbox.
    "min": 55.6,

    // Maximum value for slider or spinbox.
    "max": 100.4,

    // [REQUIRED]
    // Control is readonly i.e. values are passed
    //    [user app] -> [gui]
    // only.
    "readonly": false,

    // [OPTIONAL] Default $\frac{1000}/{(max-min)}$
    // Number of decimal places (after the period).
    "decimals": 3,


}
```

GUI shall ignore unknown fields.

## GUI Control Types

Supported GUI types are:

1. `checkbox` is typically used for boolean values.
2. `slider` is typically used for continuously adjustable values e.d. window width.
3. `spinbox` is used for discrete values of limited range e.g. integers 0x0-0xf.
4. `radiobutton` is used for enums.
