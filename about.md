# Better Messages

BetterMessages transforms the archaic email-style messaging system to a chat system.


## Usage
From anywhere, press F9 to open the chat interface. Press F9 or Esc to close.
Mobile support will be added in the future.


## Features
- Consolidates entire message history with a player into a chat tab.
- Send and read messages from a player through the same chat interface.
- Incoming messages are fetched automatically.


## Limitations
- Only supports in-game friends for now.
- Due to the way the game handles message upload times, the mod cannot provide minute-accurate message timestamps and thus will not provide any.
- The message subject is fixed to "Sent with BetterMessages".


## Future Additions
- Mobile UI (note that there is currently no button to access the chat menu on mobile)
- Other UI improvements
- Customisable message fetch intervals (currently 5s (active) /30s (background))
- Customisable keybinds
- Ability to message non-friends
- Removing deleted messages from chat history:w
- Notification support


## Credits
Special thanks to:
- [dank_meme](https://github.com/dankmeme01) and [undefined06855](https://github.com/undefined06855) for answering all my (sometimes dumb) questions about Cocos and Geode.
- [Cvolton](https://github.com/Cvolton) for providing `std::unordered_map<int, T>` serialisation to `matjson::Value`.
- [altalk23](https://github.com/altalk23) and undefined06855 for Better Touch Prio.
- Everybody else in the Geode SDK server for helping with other issues I faced.

