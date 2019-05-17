#SingleInstance force

ProjectPath = C:\Projects\2019\STNICCC_3DO\src\3DO\

IfWinExist OperaFS (De)Compiler 0.1 [altmer.arts-union.ru]
{
	WinActivate
	Send !{f4} ; Simulates the keypress alt+f4
	Run .\tools\opera\OperaFS.exe
	WinWait, OperaFS[De]Compiler,
	Sleep, 250 ; wait for 200ms
	SEND {Tab}{Space}
	Sleep, 250 ; wait for 200ms
	WinWait, Select Directory,
	SEND {Tab}%ProjectPath%CD{ENTER}
	Sleep, 250 ; wait for 200ms
	SEND %ProjectPath%demo.iso{ENTER}
	Sleep, 250 ; wait for 200ms
	Send !{f4} ; Simulates the keypress alt+f4
}
else
{
	Run .\tools\opera\OperaFS.exe
	WinWait, OperaFS (De)Compiler 0.1 [altmer.arts-union.ru],
	Sleep, 250 ; wait for 200ms
	SEND {Tab}{Space}
	Sleep, 250 ; wait for 200ms
	WinWait, Select Directory,
	SEND {Tab}%ProjectPath%CD{ENTER}{ENTER}
	Sleep, 500 ; wait for 500ms
	SEND %ProjectPath%demo.iso{ENTER}
	Sleep, 250 ; wait for 200ms
	Send !{f4} ; Simulates the keypress alt+f4
}

return
