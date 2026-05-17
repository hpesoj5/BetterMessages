# 1.0.0-beta.1
- Changed all profile and message requests to web requests instead of through RobTop's delegates
- Added rate limits (not configurable yet) to throttle outgoing requests
- Moved most functions to main thread so there should be no data race
- Used more geode utils
- Changed the F9 keybind from a CCKeyboardDispatcher hook to a keybind setting
- I'm still gonna manually add the popup to scene and set zorder

# 1.0.0-beta
- Initial release
- Will be upgraded to 1.0.0 once todo list is done
