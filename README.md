# Warning
Instant inventory loading will allow you to view your inventory, swap equipment, etc.. immediately after zoning.  It has been tested and edge cases have been considered.
But, this is still a use-at-your-own-risk situation, if you choose to enable it you should still avoid dropping items or doing anything questionable immediately after zoning.
Further, much like bellhop, you should absolutely not use this feature after a maintenance until it is verified that Ashita structures have not broken in any way.

# Summary
FindAll is a plugin that provides inventory cacheing.<br>
In addition to the obvious function of locating items on various characters, FindAll can cache your local character's inventory so that it loads instantly upon zoning.<br>
Both of these functions are disabled until you've allowed the client to fully load inventory once.
If you change to another character on the same account, you will once again have to wait until inventory fully loads for your own safety.
Note that if you have FindAll on autoload, the initial zonein counts as long as you allow inventory to fully load before zoning again.
You can search before these conditions are met, but new items will not be cached on a fresh login until inventory has fully loaded once.
Note that in addition to the typed commands, you may double click a search's tab in ImGui mode to close only that search.
For more in-depth documentation, see included html document.

# Commands
These commands should be typed directly into the FFXI console. They can all be prefixed with /fa or /findall.


**/fa config instantload [Optional: on/off/disabled/enabled]**<br>
Allows the user to toggle whether instant inventory loading will be used. If a state is not specified, will alternate.<br><br>

**/fa config cachetodisc [Optional: on/off/disabled/enabled]**<br>
Allows the user to toggle whether the current character's inventory will be backed up for searching. You can still search on a character that isn't making backups. If a state is not specified, will alternate.<br><br>

**/fa config displaymax [Required: maximum]**<br>
Allows the user to change how many search results will display in ImGui mode.  Must be between 1 and 20.<br><br>

**/fa config displaymode [Optional: imgui/chatlog]**<br>
Changes the display mode. If a mode is not specified, will alternate.<br><br>

**/fa config writedelay [Required: delay]**<br>
Changes the amount of time, in milliseconds. after an inventory change before it is cached to disc. This is used so that frequent inventory changes do not each require a disc write.  Must be between 0 and 300000.<br><br>

**/fa clear**<br>
Clears all searches that are currently displayed in the ImGui element. Does nothing in chatlog displaymode.<br><br>

**/fa search [Required: Search Term]**<br>
Effects: Searches all character caches and your local character for instances of the items and displays the results according to the current display mode.
Term can be an item ID, item name, or a wildcard string that will represent multiple item names.
Be aware that an extremely broad search(such as **/fa search \***) may crash or lag the client in chatlog mode due to the volume of data being printed.