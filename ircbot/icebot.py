#!/usr/bin/env python3
# -*- coding: latin-1 -*-

import irc.bot
import irc.strings
from irc.client import ip_numstr_to_quad, ip_quad_to_numstr

class TestBot(irc.bot.SingleServerIRCBot):
    detFunctions = {
        0: ("0", "Begin", "Sets up the graphics canvas (8bpp, default palette)."),
        1: ("1", "End", "Closes the graphics library and sets up for the TI-OS."),
        2: ("2,>rCOLOR<s", "SetColor", "Sets the global color index for all routines."),
        3: ("3", "SetDefaultPalette", "Sets up the default palette where H=L (xLIBC palette)."),
        4: ("4,>g\"DATA\"<s,>rSIZE<s,>rOFFSET<s", "SetPalette", "Sets entries in the palette. Each entry is 2 bytes, so >rSIZE<s should be the amount of entries you want to set times 2."),
        5: ("5,>rCOLOR<s", "FillScreen", "Fills the screen with the specified color index."),
        6: ("6,>rX<s,>rY<s", "SetPixel", "Sets the color pixel to the global color index."),
        7: ("7,>rX<s,>rY<s", "GetPixel", "Gets a pixel's color index."),
        8: ("8", "GetDraw", "Gets the current draw location. 0 = screen, 1 = buffer."),
        9: ("9,>rBUFFER<s", "SetDraw", "Forces drawing routines to operate on the offscreen buffer or to operate on the visible screen. 0 = draw at screen, 1 = draw at buffer."),
        10: ("10", "SwapDraw", "Safely swap the vram buffer pointers for double buffered output."),
        11: ("11,>rBUFFER<s", "Blit", "Copies the buffer image to the screen and vice versa. 0 = copy screen to buffer, 1 = copy buffer to screen."),
        12: ("12,>rBUFFER<s,>rY<s,>rLINES<s", "BlitLines", "Copies >rLINES<s lines starting at position >rY<s from the buffer to the screen or vice versa. 0 = copy screen to buffer, 1 = copy buffer to screen."),
        13: ("13,>rBUFFER<s,>rX<s,>rY<s,>rWIDTH<s,>rHEIGHT<s", "BlitArea", "Copies a specific rectangle starting at (>rX<s,>rY<s) and dimensions (>rWIDTH<s,>rHEIGHT<s) from the buffer to the screen or vice versa. 0 = copy screen to buffer, 1 = copy buffer to screen."),
        14: ("14,>rCHAR<s", "PrintChar", "Places a character at the current cursor position. Default coordinates are (0,0)."),
        15: ("15,>rEXP<s,>rCHARS<s", "PrintInt", "Places signed >rEXP<s at the current cursor position with >rCHARS<s characters. Default coordinates are (0,0)."),
        16: ("16,>rEXP<s,>rCHARS<s", "PrintUInt", "Places unsigned >rEXP<s at the current cursor position with >rCHARS<s characters. Default coordinates are (0,0)."),
        17: ("17,>g\"STRING\"<s", "PrintString", "Places a string at the current cursor position. Default coordinates are (0,0)."),
        18: ("18,>g\"STRING\"<s,>rX<s,>rY<s", "PrintStringXY", "Places a string at the given coordinates."),
        19: ("19,>rX<s,>rY<s", "SetTextXY", "Sets the coordinates for text routines."),
        20: ("20,>rCOLOR<s", "SetTextBGColor", "Sets the background text color for text routines. Default color is 255."),
        21: ("21,>rCOLOR<s", "SetTextFGColor", "Sets the foreground text color for text routines. Default color is 0."),
        22: ("22,>rCOLOR<s", "SetTextTransparentColor", "Sets the transparency text color for text routines. Default color is 255."),
        23: ("23,>g\"DATA\"<s", "SetCustomFontData", "Sets the font for all text commands. >g\"DATA\"<s should either be a string with the data, or a pointer to the data of the formated 8x8 pixel font."),
        24: ("24,>g\"DATA\"<s", "SetCustomFontSpacing", "Sets the custom font spacing for all text commands. >g\"DATA\"<s can also be a pointer to the data."),
        25: ("25,>rSPACE<s", "SetMonoSpaceFont", "Sets the font to be monospace.")
    }
    extraOutput = ""
    namedFunctions = dict((doc[1].lower(), doc) for (n, doc) in detFunctions.items())

    def __init__(self, channel, nickname, server, port=6667):
        irc.bot.SingleServerIRCBot.__init__(
            self, [(server, port)], nickname, nickname)
        self.channel = channel

    def on_nicknameinuse(self, c, e):
        c.nick(c.get_nickname() + "_")

    def on_welcome(self, c, e):
        print("Joined channels")
        c.join(self.channel)

    def on_privmsg(self, c, e):
        self.do_ice(e, e.arguments[0][5:])

    def on_pubmsg(self, c, e):
        request = e.arguments[0]
        nick = e.source.nick

        if nick == "saxjax":
            offset = request.find("]")
            request = request[offset+2:]

        if request.startswith("~ice"):
            self.do_ice(e, irc.strings.lower(request[5:]))

    def do_ice(self, e, cmd):
        c = self.connection
        output = "[>b>oICE<s<b] "
        self.extraOutput = ""

        if cmd == "" or len(cmd.split(" ")) != 1:
            output += "invalid amount of arguments; use \"~ice det(XX)\" or \"~ice sum(XX)\" or \"~ice <function>\" to search for a function"
        else:
            if cmd[-1:] == ")":
                cmd = cmd[0:-1]

            if cmd.startswith("det(") and cmd[4:].isdigit():
                det = int(float(cmd[4:]))
                
                if det < len(self.detFunctions):
                    self.func_to_string(self.detFunctions[det])
                    output += self.extraOutput
                else:
                    output += "Invalid C function"
            elif cmd.startswith("sum(") and cmd[4:].isdigit():
                det = int(float(cmd[4:]))
                output += "sum(" + str(det) + ") | "
            elif cmd.isdigit():
                det = int(float(cmd))

                if det < len(self.detFunctions):
                    self.func_to_string(self.detFunctions[det])
                    output += self.extraOutput
                else:
                    output += "Invalid C function"
            else:
                function = self.namedFunctions.get(cmd)
                
                if function is not None:
                    self.func_to_string(function)
                    output += self.extraOutput
                else:
                    output += "Unknown C function"

        if e.target != "#cemetech":
            output = output.replace(">b", "\x02")
            output = output.replace(">n", "\x0302")
            output = output.replace(">r", "\x0304")
            output = output.replace(">o", "\x0307")
            output = output.replace(">g", "\x0315")
            output = output.replace("<b", "\x02")
            output = output.replace("<s", "\x03")
        else:
            output = output.replace(">b", "")
            output = output.replace(">n", "")
            output = output.replace(">r", "")
            output = output.replace(">o", "")
            output = output.replace(">g", "")
            output = output.replace("<b", "")
            output = output.replace("<s", "")

        c.privmsg(e.target, output)

    def func_to_string(self, function):
        self.extraOutput += ">b" + function[1] + "<b | "
        self.extraOutput += ">b>ndet(<s<b"
        self.extraOutput += function[0]
        self.extraOutput += ">b>n)<s<b"
        self.extraOutput += " | " + function[2]

def main():
    server = "irc.choopa.net"
    port = 6667
    channel = "#icedev"
    nickname = "ICEbot"

    bot = TestBot(channel, nickname, server, port)
    bot.start()


if __name__ == "__main__":
    main()
