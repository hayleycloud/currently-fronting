# Currently Fronting

Lightweight utility for showing plural fronting status on Discord via the activity/presence display.

## Table of Contents
* [License](#license)
* [Usage](#usage)
    * [Introduction](#introduction)
    * [Installation](#installing)
        * [Windows](#windows)
        * [Linux](#linux)
    * [Configuration](#configuration)
        * [Core Options](#core-options)
        * [Linking To *PluralKit*](#linking-to-pluralkit)
        * [Linking To *SimplyPlural*](#linking-to-simplyplural)


## License


## Usage

### Introduction

Currently Fronting is a lightweight utility designed for plurals who wish to show their current fronting state on Discord, mainly to friends or people in DMs.

### Installation

#### Windows


#### Linux

##### Ubuntu/Debian

##### Arch Linux

##### Compiling From Source

The main build system for Currently Fronting is CMake. Currently Fronting is programmed in C, so any C compliant compiler should be sufficient.

The following commands should be sufficient to compile the project:

```bash
git clone --branch release --single-branch https://github.com/hayleycloud/currently-fronting
cd plural-frontline
cmake -S . -B build -DCMAKE_BUILD_TYPE="Release"
cmake --build build
```

You can then run the program from that directory using:

`build/fronting`

Systemd service files are provided for your convenience in the platform/linux/ subdirectory.

### Configuration

The main configuration file for user-specified settings is `config.json`. Where this is located depends on your platform:
* **Windows**: 
* **Linux**: By default, `/home/{username}/.config/currently-fronting/config.json`

#### Core Options

* `show_pronouns`: Toggles whether pronouns should be displayed with the names of the fronters (e.g. "Currently Fronting: Ren (he/they), Rey (she/her)").
* `source`: Indicates where the fronting data is being sourced from. Can be `simply_plural` for querying from *Simply Plural*, `pluralkit` for querying from *PluralKit*, or `manual` for configuring it yourselves.
* `avatar_mode`: Indicates what is used as the main activity icon. Can be:
    * `member`: Use the avatar of the fronting member, if available.
    * `system`: Use the system avatar, if available.
    * `member_sys`: Use `member`, but fall back to `system` if not available. *(Default)*
    * `app`: Use the application icon.
    * ``: Keep empty to use no avatar.
* `icon_mode`: Discord can display small icons to complement the avatar. This settings allows it to be used.
    * ``: Keep empty to use no complementary icon. *(Default)*
    * `member`: Use the fronting member avatar, if available.
* `show_fronting_time`: Show the fronting time, if possible. *`false` by default.*

[icon modes examples]

#### Linking To *PluralKit*

This section provides configuration for linking to *PluralKit* system profiles.

##### Generating An Access Token

First, Currently Fronting needs a unique token in order to access your PluralKit instance. To do this, open a DM with PluralKit, and send it "pk;token". **Do not contact PluralKit with this in a server with others! This could compromise the security of your instance!** PluralKit will generate a token for you, which will look like this:

[example img]

You will need to copy this string of madness from the response, and paste it into the `token` field of the PluralKit part of the configuration file.

[example img]

#### Linking To *Simply Plural*

This section provides configuration for linking to *Simply Plural* accounts.

##### Generating An Access Token

First, Currently Fronting needs a token in order to access your Simply Plural account. To do this, open Simply Plural and go to "**Tokens**". Ideally, you should do this step on the machine that's running Currently Fronting, to make copying the token easier.

[example img]

Then click "Add Token" to generate a new token.

[example img]

You will want to only check the "Read" option. Currently Fronting only needs to read; it doesn't need to change anything in your profile.

[example img]

Then copy this string of terrors into the `token` field of the Simply Plural part of the configuration file.

[example img]


