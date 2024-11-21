Scriptname CustomConsole Hidden

; Print the given message into the console
Function PrintConsole(String asText) native global

; Execute the given command in the console
Function ExecuteCommand(String asCommand, ObjectReference akTargetReference) native global

; Get the last n console messages. The stack has a maximum size of 128 messages
String[] Function GetConsoleMessages(int n) native global

; Register to receive an event if the specified console command is used
; If partial match is true, you will receive any command that contains the specified command, otherwise only exact matches will be received
; Leading and trailing whitespaces are ignored
Function RegisterForConsoleCommand(Form akForm, String asCommand, bool abPartialMatch) native global
Function RegisterForConsoleCommand_Alias(Alias akAlias, String asCommand, bool abPartialMatch) native global
Function RegisterForConsoleCommand_MgEff(MagicEffect akMagicEffect, String asCommand, bool abPartialMatch) native global
Function UnregisterForConsoleCommand(Form akForm, String asCommand) native global
Function UnregisterForConsoleCommand_Alias(Alias akAlias, String asCommand) native global
Function UnregisterForConsoleCommand_MgEff(MagicEffect akMagicEffect, String asCommand) native global

Event OnConsoleCommand(String asCommand, ObjectReference akTargetReference)
EndEvent
