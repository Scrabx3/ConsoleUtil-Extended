Scriptname CustomConsole Hidden

; Print the given message into the console
Function PrintConsole(String asText) native global

; Execute the given command in the console
Function ExecuteCommand(String asCommand, ObjectReference akTargetReference) native global

; Get the last n console messages. The stack has a maximum size of 128 messages
String[] Function GetConsoleMessages(int n) native global

; Register to receive an event if the specified console command is used
; If partial match is true, you will receive any command that contains the specified command, otherwise only exact matches will be received
; The target reference is optional, if specified, the event will only be sent if the specified target is selected
; targeted commands such as "player.additem f 100" are treated as "additem f 100" with akTargetRef == player
bool Function RegisterForConsoleCommand(Form akForm, String asCommand, bool abPartialMatch, ObjectReference akTargetRef) native global
bool Function RegisterForConsoleCommand_Alias(Alias akAlias, String asCommand, bool abPartialMatch, ObjectReference akTargetRef) native global
bool Function RegisterForConsoleCommand_MgEff(ActiveMagicEffect akMagicEffect, String asCommand, bool abPartialMatch, ObjectReference akTargetRef) native global
Function UnregisterForConsoleCommand(Form akForm, String asCommand, ObjectReference akTarget) native global
Function UnregisterForConsoleCommand_Alias(Alias akAlias, String asCommand, ObjectReference akTarget) native global
Function UnregisterForConsoleCommand_MgEff(ActiveMagicEffect akMagicEffect, String asCommand, ObjectReference akTarget) native global

Event OnConsoleCommand(String asCommand, ObjectReference akTargetReference)
EndEvent
