/**
 * @file
 * @authors Matt "MateoConLechuga" Waltz
 * @authors Jacob "jacobly" Young
 * @brief Core CE define file used for computer porting
 */

#ifndef H_TICE
#define H_TICE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Colors used by the OS
 */
typedef enum {
    OS_COLOR_BLUE = 10,
    OS_COLOR_RED,
    OS_COLOR_BLACK,
    OS_COLOR_MAGENTA,
    OS_COLOR_GREEN,
    OS_COLOR_ORANGE,
    OS_COLOR_BROWN,
    OS_COLOR_NAVY,
    OS_COLOR_LTBLUE,
    OS_COLOR_YELLOW,
    OS_COLOR_WHITE,
    OS_COLOR_LTGRAY,
    OS_COLOR_MEDGRAY,
    OS_COLOR_GRAY,
    OS_COLOR_DARKGRAY
} os_colors_t;

#define os_RamStart          ((uint8_t*)0xD00000)
#define os_Flags             ((uint8_t*)0xD00080)
#define os_TextFlags         (*(uint8_t*)0xD00080)
#define os_ApdFlags          (*(uint8_t*)0xD00088)
#define os_RclFlags          (*(uint8_t*)0xD0008E)

#define os_AppData           ((uint8_t*)0xD00429)

#define os_KbdScanCode       (*(uint8_t*)0xD00587)
#define os_KbdLGSC           (*(uint8_t*)0xD00588)
#define os_KbdPSC            (*(uint8_t*)0xD00589)
#define os_KbdWUR            (*(uint8_t*)0xD0058A)
#define os_KbdDebncCnt       (*(uint8_t*)0xD0058B)
#define os_KbdKey            (*(uint8_t*)0xD0058C)
#define os_KbdGetKy          (*(uint8_t*)0xD0058D)
#define os_KeyExtend         (*(uint8_t*)0xD0058E)
#define os_Brightness        (*(uint8_t*)0xD0058F)
#define os_ApdSubTimer       (*(uint8_t*)0xD00590)
#define os_ApdTimer          (*(uint8_t*)0xD00591)
#define os_CurRow            (*(uint8_t*)0xD00595)
#define os_CurCol            (*(uint8_t*)0xD00596)

#define os_OP1               ((uint8_t*)0xD005F8)
#define os_OP2               ((uint8_t*)0xD00603)
#define os_OP3               ((uint8_t*)0xD0060E)
#define os_OP4               ((uint8_t*)0xD00619)
#define os_OP5               ((uint8_t*)0xD00624)
#define os_OP6               ((uint8_t*)0xD0062F)
#define os_OP7               ((uint8_t*)0xD0063A)

#define os_ProgToEdit        ((char*)0xD0065B)
#define os_NameBuff          ((char*)0xD00663)

#define os_TextShadow        ((uint8_t*)0xD006C0)          /**< Text buffer 260 bytes */

#define os_PromptRow         (*(uint8_t*)0xD00800)
#define os_PromptCol         (*(uint8_t*)0xD00801)
#define os_PromptIns         (*(uint8_t*)0xD00802)
#define os_PromptShift       (*(uint8_t*)0xD00803)
#define os_PromptRet         (*(uint8_t*)0xD00804)
#define os_PromptValid       (*(uint8_t*)0xD00807)

#define os_PenCol            (*(uint24_t*)0xD008D2)        /**< Small font column location */
#define os_PenRow            (*(uint8_t*)0xD008D5)         /**< Small font row location */

#define os_StatVars          ((uint8_t*)0xD01191)
#define os_AsmPrgmSize       (*(uint16_t*)0xD0118C)        /**< Current size of executing program */

#define os_uXMin             (*(real_t*)0xD01D61)
#define os_uXMax             (*(real_t*)0xD01D6A)
#define os_uXScl             (*(real_t*)0xD01D73)
#define os_uYMin             (*(real_t*)0xD01D7C)
#define os_uYMax             (*(real_t*)0xD01D85)
#define os_uYScl             (*(real_t*)0xD01D8E)
#define os_uThetaMin         (*(real_t*)0xD01D97)
#define os_uThetaMax         (*(real_t*)0xD01DA0)
#define os_uThetaStep        (*(real_t*)0xD01DA9)
#define os_uTmin             (*(real_t*)0xD01DB2)
#define os_uTmax             (*(real_t*)0xD01DBB)
#define os_uTStep            (*(real_t*)0xD01DC4)
#define os_uPlotStart        (*(real_t*)0xD01DCD)
#define os_unMax             (*(real_t*)0xD01DD6)
#define os_uu0               (*(real_t*)0xD01DDF)
#define os_uv0               (*(real_t*)0xD01DE8)
#define os_unMin             (*(real_t*)0xD01DF1)
#define os_uu02              (*(real_t*)0xD01DFA)
#define os_uv02              (*(real_t*)0xD01E03)
#define os_uw0               (*(real_t*)0xD01E0C)
#define os_uPlotStep         (*(real_t*)0xD01E15)
#define os_uXres             (*(real_t*)0xD01E1E)
#define os_uw02              (*(real_t*)0xD01E27)
#define os_XMin              (*(real_t*)0xD01E33)
#define os_XMax              (*(real_t*)0xD01E3C)
#define os_XScl              (*(real_t*)0xD01E45)
#define os_YMin              (*(real_t*)0xD01E4E)
#define os_YMax              (*(real_t*)0xD01E57)
#define os_YScl              (*(real_t*)0xD01E60)
#define os_ThetaMin          (*(real_t*)0xD01E69)
#define os_ThetaMax          (*(real_t*)0xD01E72)
#define os_ThetaStep         (*(real_t*)0xD01E7B)
#define os_TMinPar           (*(real_t*)0xD01E84)
#define os_TMaxPar           (*(real_t*)0xD01E8D)
#define os_TStep             (*(real_t*)0xD01E96)
#define os_PlotStart         (*(real_t*)0xD01E9F)
#define os_NMax              (*(real_t*)0xD01EA8)
#define os_u0                (*(real_t*)0xD01EB1)
#define os_V0                (*(real_t*)0xD01EBA)
#define os_NMin              (*(real_t*)0xD01EC3)
#define os_u02               (*(real_t*)0xD01ECC)
#define os_V02               (*(real_t*)0xD01ED5)
#define os_W0                (*(real_t*)0xD01EDE)
#define os_PlotStep          (*(real_t*)0xD01EE7)
#define os_XResO             (*(real_t*)0xD01EF0)
#define os_W02               (*(real_t*)0xD01EF9)
#define os_un1               (*(real_t*)0xD01F02)
#define os_un2               (*(real_t*)0xD01F0B)
#define os_Vn1               (*(real_t*)0xD01F14)
#define os_Vn2               (*(real_t*)0xD01F1D)
#define os_Wn1               (*(real_t*)0xD01F26)
#define os_Wn2               (*(real_t*)0xD01F2F)
#define os_Fin_N             (*(real_t*)0xD01F38)
#define os_Fin_I             (*(real_t*)0xD01F41)
#define os_Fin_PV            (*(real_t*)0xD01F4A)
#define os_Fin_PMT           (*(real_t*)0xD01F53)
#define os_Fin_FV            (*(real_t*)0xD01F5C)
#define os_Fin_PY            (*(real_t*)0xD01F65)
#define os_Fin_CY            (*(real_t*)0xD01F6E)
#define os_Cal_N             (*(real_t*)0xD01F77)
#define os_Cal_I             (*(real_t*)0xD01F80)
#define os_Cal_PV            (*(real_t*)0xD01F89)
#define os_Cal_PMT           (*(real_t*)0xD01F92)
#define os_Cal_FV            (*(real_t*)0xD01F9B)
#define os_Cal_PY            (*(real_t*)0xD01FA4)

#define os_CmdShadow         ((uint8_t*)0xD0232D)

#define os_Y1LineType        (*(uint8_t*)0xD024BF)
#define os_Y2LineType        (*(uint8_t*)0xD024C0)
#define os_Y3LineType        (*(uint8_t*)0xD024C1)
#define os_Y4LineType        (*(uint8_t*)0xD024C2)
#define os_Y5LineType        (*(uint8_t*)0xD024C3)
#define os_Y6LineType        (*(uint8_t*)0xD024C4)
#define os_Y7LineType        (*(uint8_t*)0xD024C5)
#define os_Y8LineType        (*(uint8_t*)0xD024C6)
#define os_Y9LineType        (*(uint8_t*)0xD024C7)
#define os_Y0LineType        (*(uint8_t*)0xD024C8)
#define os_Para1LineType     (*(uint8_t*)0xD024C9)
#define os_Para2LineType     (*(uint8_t*)0xD024CA)
#define os_Para3LineType     (*(uint8_t*)0xD024CB)
#define os_Para4LineType     (*(uint8_t*)0xD024CC)
#define os_Para5LineType     (*(uint8_t*)0xD024CD)
#define os_Para6LineType     (*(uint8_t*)0xD024CE)
#define os_Polar1LineType    (*(uint8_t*)0xD024CF)
#define os_Polar2LineType    (*(uint8_t*)0xD024D0)
#define os_Polar3LineType    (*(uint8_t*)0xD024D1)
#define os_Polar4LineType    (*(uint8_t*)0xD024D2)
#define os_Polar5LineType    (*(uint8_t*)0xD024D3)
#define os_Polar6LineType    (*(uint8_t*)0xD024D4)
#define os_SecULineType      (*(uint8_t*)0xD024D5)
#define os_SecVLineType      (*(uint8_t*)0xD024D6)
#define os_SecWLineType      (*(uint8_t*)0xD024D7)
#define os_Y1LineColor       (*(uint8_t*)0xD024D8)
#define os_Y2LineColor       (*(uint8_t*)0xD024D9)
#define os_Y3LineColor       (*(uint8_t*)0xD024DA)
#define os_Y4LineColor       (*(uint8_t*)0xD024DB)
#define os_Y5LineColor       (*(uint8_t*)0xD024DC)
#define os_Y6LineColor       (*(uint8_t*)0xD024DD)
#define os_Y7LineColor       (*(uint8_t*)0xD024DE)
#define os_Y8LineColor       (*(uint8_t*)0xD024DF)
#define os_Y9LineColor       (*(uint8_t*)0xD024E0)
#define os_Y0LineColor       (*(uint8_t*)0xD024E1)
#define os_Para1LineColor    (*(uint8_t*)0xD024E2)
#define os_Para2LineColor    (*(uint8_t*)0xD024E3)
#define os_Para3LineColor    (*(uint8_t*)0xD024E4)
#define os_Para4LineColor    (*(uint8_t*)0xD024E5)
#define os_Para5LineColor    (*(uint8_t*)0xD024E6)
#define os_Para6LineColor    (*(uint8_t*)0xD024E7)
#define os_Polar1LineColor   (*(uint8_t*)0xD024E8)
#define os_Polar2LineColor   (*(uint8_t*)0xD024E9)
#define os_Polar3LineColor   (*(uint8_t*)0xD024EA)
#define os_Polar4LineColor   (*(uint8_t*)0xD024EB)
#define os_Polar5LineColor   (*(uint8_t*)0xD024EC)
#define os_Polar6LineColor   (*(uint8_t*)0xD024ED)
#define os_SecULineColor     (*(uint8_t*)0xD024EE)
#define os_SecVLineColor     (*(uint8_t*)0xD024EF)
#define os_SecWLineColor     (*(uint8_t*)0xD024F0)

#define os_AppErr1           ((char*)0xD025A9)             /**< String [1] for custom error */
#define os_AppErr2           ((char*)0xD025B6)             /**< String [2] for custom error */

#define os_CursorHookPtr     (*(uint24_t*)0xD025D5)
#define os_LibraryHookPtr    (*(uint24_t*)0xD025D8)
#define os_RawKeyHookPtr     (*(uint24_t*)0xD025DB)
#define os_GetKeyHookPtr     (*(uint24_t*)0xD025DE)
#define os_HomescreenHookPtr (*(uint24_t*)0xD025E1)
#define os_WindowHookPtr     (*(uint24_t*)0xD025E4)
#define os_GraphHookPtr      (*(uint24_t*)0xD025E7)
#define os_YEqualsHookPtr    (*(uint24_t*)0xD025EA)
#define os_FontHookPtr       (*(uint24_t*)0xD025ED)
#define os_RegraphHookPtr    (*(uint24_t*)0xD025F0)
#define os_GraphicsHookPtr   (*(uint24_t*)0xD025F3)
#define os_TraceHookPtr      (*(uint24_t*)0xD025F6)
#define os_ParserHookPtr     (*(uint24_t*)0xD025F9)
#define os_AppChangeHookPtr  (*(uint24_t*)0xD025FC)
#define os_Catalog1HookPtr   (*(uint24_t*)0xD025FF)
#define os_HelpHookPtr       (*(uint24_t*)0xD02602)
#define os_CxRedispHookPtr   (*(uint24_t*)0xD02605)
#define os_MenuHookPtr       (*(uint24_t*)0xD02608)
#define os_Catalog2HookPtr   (*(uint24_t*)0xD0260B)
#define os_TokenHookPtr      (*(uint24_t*)0xD0260E)
#define os_LocalizeHookPtr   (*(uint24_t*)0xD02611)
#define os_SilentLinkHookPtr (*(uint24_t*)0xD02614)
#define os_ActiveUSBHookPtr  (*(uint24_t*)0xD0261A)

#define os_TempFreeArc       (*(uint24_t*)0xD02655)        /**< Set after asm_ArcChk call */

#define os_TextBGcolor       (*(uint16_t*)0xD02688)        /**< Large font background color */
#define os_TextFGcolor       (*(uint16_t*)0xD0268A)        /**< Large font foreground color */

#define os_DrawBGColor       (*(uint16_t*)0xD026AA)        /**< Small font background color */
#define os_DrawFGColor       (*(uint16_t*)0xD026AC)        /**< Small font foreground color */
#define os_DrawColorCode     (*(uint8_t*)0xD026AE)

#define os_BatteryStatus     (*(uint8_t*)0xD02A86)

#define os_GraphBGColor      (*(uint16_t*)0xD02A98)        /**< Graph background 565 color */

#define os_FillRectColor     (*(uint16_t*)0xD02AC0)
#define os_StatusBarBGColor  (*(uint16_t*)0xD02ACC)        /**< Status bar 565 color */

#define os_TmpYears          (*(real_t*)0xD02B01)
#define os_TmpMonths         (*(real_t*)0xD02B0A)
#define os_TmpDays           (*(real_t*)0xD02B13)
#define os_TmpHours          (*(real_t*)0xD02B1C)
#define os_TmpMinutes        (*(real_t*)0xD02B25)

#define os_RamCode           ((uint8_t*)0xD18C7C)          /**< 1023 bytes free ram */

/*
 * TI-OS Token definitions
 */

#define tToDMS              0x01
#define tToDEC              0x02
#define tToAbc              0x03
#define tStore              0x04
#define tBoxPlot            0x05
#define tLBrack             0x06
#define tRBrack             0x07
#define tLBrace             0x08
#define tRBrace             0x09
#define tFromRad            0x0A
#define tFromDeg            0x0B
#define tRecip              0x0C
#define tSqr                0x0D
#define tTrnspos            0x0E
#define tCube               0x0F
#define tLParen             0x10
#define tRParen             0x11
#define tRound              0x12
#define tPxTst              0x13
#define tAug                0x14
#define tRowSwap            0x15
#define tRowPlu             0x16
#define tmRow               0x17
#define tmRowPlus           0x18
#define tMax                0x19 // 'max'
#define tMin                0x1A // 'min'
#define tRToPr              0x1B // 'R>Pr'
#define tRToPo              0x1C // 'R>Po'
#define tPToRx              0x1D // 'P>Rx'
#define tPToRy              0x1E // 'P>Ry'
#define tMedian             0x1F // 'MEDIAN'
#define tRandM              0x20 // 'randM'
#define tMean               0x21 // 'MEAN'
#define tRoot               0x22 // 'ROOT'
#define tSeries             0x23 // 'seq'
#define tFnInt              0x24 // 'fnInt'
#define tNDeriv             0x25 // 'fnIr'
#define tEvalF              0x26
#define tFmin               0x27
#define tFmax               0x28
#define tSpace              0x29 // ' '
#define tString             0x2A // '"'
#define tComma              0x2B // ','
#define tii                 0x2C // 'i'
#define tFact               0x2D // '!'
#define tCubicR             0x2E
#define tQuartR             0x2F
#define t0                  0x30
#define t1                  0x31
#define t2                  0x32
#define t3                  0x33
#define t4                  0x34
#define t5                  0x35
#define t6                  0x36
#define t7                  0x37
#define t8                  0x38
#define t9                  0x39
#define tDecPt              0x3A // '.'
#define tee                 0x3B // 'e'
#define tOr                 0x3C // '_or_'
#define tXor                0x3D
#define tColon              0x3E // ':'
#define tEnter              0x3F
#define tAnd                0x40 // '_and_'
#define tA                  0x41
#define tB                  0x42
#define tC                  0x43
#define tD                  0x44
#define tE                  0x45
#define tF                  0x46
#define tG                  0x47
#define tH                  0x48
#define tI                  0x49
#define tJ                  0x4A
#define tK                  0x4B
#define tL                  0x4C
#define tM                  0x4D
#define tN                  0x4E
#define tO                  0x4F
#define tP                  0x50
#define tQ                  0x51
#define tR                  0x52
#define tS                  0x53
#define tT                  0x54
#define tU                  0x55
#define tV                  0x56
#define tW                  0x57
#define tX                  0x58
#define tY                  0x59
#define tZ                  0x5A
#define tTheta              0x5B

/*
 * Extended Tokens
 */
#define tExtTok             0xEF
#define tSetDate            0x00
#define tSetTime            0x01
#define tCheckTmr           0x02
#define tSetDtFmt           0x03
#define tSetTmFmt           0x04
#define tTimeCnv            0x05
#define tDayOfWk            0x06
#define tGetDtStr           0x07
#define tGetTmStr           0x08
#define tGetDate            0x09
#define tGetTime            0x0A
#define tStartTmr           0x0B
#define tGtDtFmt            0x0C
#define tGetTmFmt           0x0D
#define tIsClockOn          0x0E
#define tClockOff           0x0F
#define tClockOn            0x10
#define tOpenLib            0x11
#define tExecLib            0x12
#define tInvT               0x13
#define tChiSquaredTest     0x14
#define tLinRegTInt         0x15
#define tManualFit          0x16
#define tZQuadrant          0x17
#define tZFracHalf          0x18
#define tZFracThird         0x19
#define tZFracFourth        0x1A
#define tZFracFifth         0x1B
#define tZFracEighth        0x1C
#define tZFracTenth         0x1D
#define tFracSlash          0x2E
#define tFracMixedNum       0x2F
#define tSwapImProper       0x30
#define tSwapFracDec        0x31
#define tRemainder          0x32
#define tSummationSigma     0x33
#define tLogBase            0x34
#define tRandIntNoRep       0x35
#define tMathPrint          0x36
#define tClassic            0x38
#define tAutoAnswer         0x3B
#define tDecAnswer          0x3C
#define tFracAnswer         0x3D
#define tBlue               0x41
#define tRed                0x42
#define tBlack              0x43
#define tMagenta            0x44
#define tGreen              0x45
#define tOrange             0x46
#define tBrown              0x47
#define tNavy               0x48
#define tLtBlue             0x49
#define tYellow             0x4A
#define tWhite              0x4B
#define tLtGray             0x4C
#define tMedGray            0x4D
#define tGray               0x4E
#define tDarkGray           0x4F
#define tGraphColor         0x65
#define tTextColor          0x67
#define tBackgroundOn       0x5B
#define tBackgroundOff      0x64
#define tThin               0x74
#define tBorderColor        0x6C
#define tAsm84CPrgm         0x68
#define tAsm84CCmp          0x69
#define tAsm84CeCmp         0x7B
#define tAsm84CePrgm        0x7A

#define tVarMat             0x5C
#define tVarLst             0x5D
#define tVarEqu             0x5E
#define tProg               0x5F
#define tVarPict            0x60
#define tVarGDB             0x61
#define tVarOut             0x62
#define tVarSys             0x63

/*
 * Mode settings tokens
 */
#define tRad                0x64 // 'Radian'
#define tDeg                0x65 // 'Degree'
#define tNormF              0x66 // 'Normal'
#define tSci                0x67 // 'Sci'
#define tEng                0x68 // 'Eng'
#define tFloat              0x69 // 'Float'
#define tFix                0x73 // 'Fix_'
#define tSplitOn            0x74
#define tFullScreen         0x75
#define tStndrd             0x76 // 'Func'
#define tParam              0x77 // 'Param'
#define tPolar              0x78 // 'Pol'
#define tSeqG               0x79 // ;79h
#define tAFillOn            0x7A // 'AUTO FILL ON'
#define tAFillOff           0x7B // 'AutoFill OFF'
#define tACalcOn            0x7C
#define tACalcOff           0x7D

#define tEQ                 0x6A // '='
#define tLT                 0x6B // '<'
#define tGT                 0x6C // '>'
#define tLE                 0x6D // LLE
#define tGE                 0x6E // LGE
#define tNE                 0x6F // LNE
#define tAdd                0x70 // '+'
#define tSub                0x71 // '-'
#define tAns                0x72
#define tMul                0x82 // '*'
#define tDiv                0x83 // '/'

#define tBoxIcon            0x7F
#define tCrossIcon          0x80
#define tDotIcon            0x81

#define tTrace              0x84 // 'Trace'
#define tClDrw              0x85 // 'ClrDraw'
#define tZoomStd            0x86 // 'ZStd'
#define tZoomtrg            0x87 // 'Ztrg'
#define tZoomBox            0x88 // 'ZBOX'
#define tZoomIn             0x89 // 'ZIn'
#define tZoomOut            0x8A // 'ZOut'
#define tZoomSqr            0x8B // 'ZSqr'
#define tZoomInt            0x8C // 'ZInt'
#define tZoomPrev           0x8D // 'ZPrev'
#define tZoomDec            0x8E // 'ZDecm'
#define tZoomStat           0x8F // 'ZStat
#define tUsrZm              0x90 // 'ZRcl'
#define tPrtScrn            0x91 // 'PrtScrn'
#define tZoomSto            0x92 //  'ZSto'
#define tText               0x93

#define tnPr                0x94 // '_nPr_'
#define tnCr                0x95 // '_nCr_'

// Graph Commands
#define tYOn                0x96 // 'FnOn_'
#define tYOff               0x97 // 'FnOff_'
#define tStPic              0x98 // 'StPic_'
#define tRcPic              0x99 // 'RcPic_'
#define tStoDB              0x9A // 'StGDB_'
#define tRclDB              0x9B // 'RcGDB_'
#define tLine               0x9C // 'Line'
#define tVert               0x9D // 'Vert_'
#define tPtOn               0x9E // 'PtOn'
#define tPtOff              0x9F // 'PtOff'
#define tPtChg              0xA0 // 'PtChg'
#define tPXOn               0xA1
#define tPXOff              0xA2
#define tPXChg              0xA3
#define tShade              0xA4 // 'Shade'
#define tCircle             0xA5 // 'Circle'
#define tHorz               0xA6 // 'HORIZONTAL'
#define tTanLn              0xA7 // 'TanLn'
#define tDrInv              0xA8 // 'DrInv_'
#define tDrawF              0xA9 // 'DrawF_'
#define tVarStrng           0xAA

// Functions with no argument
#define tRand               0xAB // 'rand'
#define tPi                 0xAC //  Lpi
#define tGetKey             0xAD // 'getKy'
#define tAPost              0xAE // '''
#define tQuest              0xAF // '?'
#define tChs                0xB0
#define tInt                0xB1
#define tAbs                0xB2
#define tDet                0xB3
#define tIdent              0xB4
#define tDim                0xB5
#define tSum                0xB6
#define tProd               0xB7
#define tNot                0xB8
#define tIPart              0xB9
#define tFPart              0xBA

// New 2 Byte Tokens
#define t2ByteTok           0xBB
#define tSqrt               0xBC
#define tCubRt              0xBD
#define tLn                 0xBE
#define tExp                0xBF
#define tLog                0xC0
#define tALog               0xC1
#define tSin                0xC2
#define tASin               0xC3
#define tCos                0xC4
#define tACos               0xC5
#define tTan                0xC6
#define tATan               0xC7
#define tSinH               0xC8
#define tASinH              0xC9
#define tCoshH              0xCA
#define tACosH              0xCB
#define tTanH               0xCC
#define tATanH              0xCD

// Some Programming Commands
#define tIf                 0xCE // 'If_'
#define tThen               0xCF // 'Then_'
#define tElse               0xD0 // 'Else_'
#define tWhile              0xD1 // 'While_'
#define tRepeat             0xD2 // 'Repeat_'
#define tFor                0xD3 // 'For_'
#define tEnd                0xD4 // 'End'
#define tReturn             0xD5 // 'Return'
#define tLbl                0xD6 // 'Lbl_'
#define tGoto               0xD7 // 'Goto_'
#define tPause              0xD8 // 'Pause_'
#define tStop               0xD9 // 'Stop'
#define tISG                0xDA // 'IS>'
#define tDSL                0xDB // 'DS<'
#define tInput              0xDC // 'Input_'
#define tPrompt             0xDD // 'Prompt_'
#define tDisp               0xDE // 'Disp_'
#define tDispG              0xDF // 'DispG'
#define tOutput             0xE0 // 'Outpt'
#define tClLCD              0xE1 // 'ClLCD'
#define tConst              0xE2 // 'Fill'
#define tSortA              0xE3 // 'sortA_'
#define tSortD              0xE4 // 'sortD_'
#define tDispTab            0xE5 // 'Disp Table
#define tMenu               0xE6 // 'Menu'
#define tSendMBL            0xE7 // 'Send'
#define tGetMBL             0xE8 // 'Get'

// Stat Plot Commands
#define tPlotOn             0xE9 // 'PLOTSON'
#define tPlotOff            0xEA // 'PLOTSOFF
#define tListName           0xEB // List Designator
#define tPlot1              0xEC
#define tPlot2              0xED
#define tPlot3              0xEE
#define tUnused01           0xEF // available?
#define tPower              0xF0 // '^'
#define tXRoot              0xF1 // LsupX,Lroot
#define tOneVar             0xF2 // 'OneVar_'
#define tTwoVar             0xF3
#define tLR                 0xF4 // 'LinR(A+BX)'
#define tLRExp              0xF5 // 'ExpR_'
#define tLRLn               0xF6 // 'LnR_'
#define tLRPwr              0xF7 // 'PwrR_'
#define tMedMed             0xF8
#define tQuad               0xF9
#define tClrLst             0xFA // 'Clear List'
#define tClrTbl             0xFB // 'Clear Table'
#define tHist               0xFC // 'Hist_'
#define txyLine             0xFD // 'xyline_'
#define tScatter            0xFE // 'Scatter_'
#define tLR1                0xFF // 'LINR(AX+B)'

// 2nd Half Of Graph Format Tokens
#define tSeq                0x00 // 'SeqG'
#define tSimulG             0x01 // 'SimulG'
#define tPolarG             0x02 // 'PolarGC'
#define tRectG              0x03 // 'RectGC'
#define tCoordOn            0x04 // 'CoordOn'
#define tCoordOff           0x05 // 'CoordOff'
#define tDrawLine           0x06 // 'DrawLine'
#define tDrawDot            0x07 // 'DrawDot'
#define tAxisOn             0x08 // 'AxesOn'
#define tAxisOff            0x09 // 'AxesOff'
#define tGridOn             0x0A // 'GridOn'
#define tGridOff            0x0B // 'GridOff'
#define tLblOn              0x0C // 'LabelOn'
#define tLblOff             0x0D // 'LabelOff'
#define tWebOn              0x0E // 'WebOn'
#define tWebOff             0x0F // 'WebOFF'
#define tuv                 0x10 // U vs V
#define tvw                 0x11 // V vs W
#define tuw                 0x12 // U vs W

// 2nd Half Of User Matrix Tokens
#define tMatA               0x00 // MAT A
#define tMatB               0x01 // MAT B
#define tMatC               0x02 // MAT C
#define tMatD               0x03 // MAT D
#define tMatE               0x04 // MAT E
#define tMatF               0x05 // MAT F
#define tMatG               0x06 // MAT G
#define tMatH               0x07 // MAT H
#define tMatI               0x08 // MAT I
#define tMatJ               0x09 // MAT J

// 2nd Half Of User List Tokens
#define tL1                 0x00 // LIST 1
#define tL2                 0x01 // LIST 2
#define tL3                 0x02 // LIST 3
#define tL4                 0x03 // LIST 4
#define tL5                 0x04 // LIST 5
#define tL6                 0x05 // LIST 6

// 2nd Half Of User Equation Tokens
// Y Equations have bit 4 set
#define tY1                 0x10 // Y1
#define tY2                 0x11 // Y2
#define tY3                 0x12 // Y3
#define tY4                 0x13 // Y4
#define tY5                 0x14 // Y5
#define tY6                 0x15 // Y6
#define tY7                 0x16 // Y7
#define tY8                 0x17 // Y8
#define tY9                 0x18 // Y9
#define tY0                 0x19 // Y0

// Param Equations Have Bit 5 Set
#define tX1T                0x20 // X1t
#define tY1T                0x21 // Y1t
#define tX2T                0x22 // X2t
#define tY2T                0x23 // Y2t
#define tX3T                0x24 // X3t
#define tY3T                0x25 // Y3t
#define tX4T                0x26 // X4t
#define tY4T                0x27 // Y4t
#define tX5T                0x28 // X5t
#define tY5T                0x29 // Y5t
#define tX6T                0x2A // X6t
#define tY6T                0x2B // Y6t

// Polar Equations Have Bit 6 Set
#define tR1                 0x40 // R1
#define tR2                 0x41 // R2
#define tR3                 0x42 // R3
#define tR4                 0x43 // R4
#define tR5                 0x44 // R5
#define tR6                 0x45 // R6

// Recursion Equations Have Bit 7 Set
#define tun                 0x80 // Un
#define tvn                 0x81 // Vn
#define twn                 0x82 // Wn

// 2nd Half User Picture Tokens
#define tPic1               0x00 // PIC1
#define tPic2               0x01 // PIC2
#define tPic3               0x02 // PIC3
#define tPic4               0x03 // PIC4
#define tPic5               0x04 // PIC5
#define tPic6               0x05 // PIC6
#define tPic7               0x06 // PIC7
#define tPic8               0x07 // PIC8
#define tPic9               0x08 // PIC9
#define tPic0               0x09 // PIC0

// 2nd Half User Graph Database Tokens
#define tGDB1               0x00 // GDB1
#define tGDB2               0x01 // GDB2
#define tGDB3               0x02 // GDB3
#define tGDB4               0x03 // GDB4
#define tGDB5               0x04 // GDB5
#define tGDB6               0x05 // GDB6
#define tGDB7               0x06 // GDB7
#define tGDB8               0x07 // GDB8
#define tGDB9               0x08 // GDB9
#define tGDB0               0x09 // GDB0

// 2nd Half Of String Vars
#define tStr1               0x00
#define tStr2               0x01
#define tStr3               0x02
#define tStr4               0x03
#define tStr5               0x04
#define tStr6               0x05
#define tStr7               0x06
#define tStr8               0x07
#define tStr9               0x08
#define tStr0               0x09

// 2nd Half Of System Output Only Variables
#define tRegEq              0x01 // REGRESSION EQUATION
#define tStatN              0x02 // STATISTICS N
#define tXMean              0x03 // X MEAN
#define tSumX               0x04 // SUM(X)
#define tSumXSqr            0x05 // SUM(X^2)
#define tStdX               0x06 // STANDARD DEV X
#define tStdPX              0x07 // STANDARD DEV POP X
#define tMinX               0x08 // Min X VALUE
#define tMaxX               0x09 // Max X VALUE
#define tMinY               0x0A // Min Y VALUE
#define tMaxY               0x0B // Max Y VALUE
#define tYmean              0x0C // Y MEAN
#define tSumY               0x0D // SUM(Y)
#define tSumYSqr            0x0E // SUM(Y^2)
#define tStdY               0x0F // STANDARD DEV Y
#define tStdPY              0x10 // STANDARD DEV POP Y
#define tSumXY              0x11 // SUM(XY)
#define tCorr               0x12 // CORRELATION
#define tMedX               0x13 // MED(X)
#define tQ1                 0x14 // 1ST QUADRANT OF X
#define tQ3                 0x15 // 3RD QUADRANT OF X
#define tQuadA              0x16 // 1ST TERM OF QUAD POLY REG/ Y-INT
#define tQuadB              0x17 // 2ND TERM OF QUAD POLY REG/ SLOPE
#define tQuadC              0x18 // 3RD TERM OF QUAD POLY REG
#define tCubeD              0x19 // 4TH TERM OF CUBIC POLY REG
#define tQuartE             0x1A // 5TH TERM OF QUART POLY REG
#define tMedX1              0x1B // x1 FOR MED-MED
#define tMedX2              0x1C // x2 FOR MED-MED
#define tMedX3              0x1D // x3 FOR MED-MED
#define tMedY1              0x1E // y1 FOR MED-MED
#define tMedY2              0x1F // y2 FOR MED-MED
#define tMedY3              0x20 // y3 FOR MED-MED
#define tRecurn             0x21 // RECURSION N
#define tStatP              0x22
#define tStatZ              0x23
#define tStatT              0x24
#define tStatChi            0x25
#define tStatF              0x26
#define tStatDF             0x27
#define tStatPhat           0x28
#define tStatPhat1          0x29
#define tStatPhat2          0x2A
#define tStatMeanX1         0x2B
#define tStatStdX1          0x2C
#define tStatN1             0x2D
#define tStatMeanX2         0x2E
#define tStatStdX2          0x2F
#define tStatN2             0x30
#define tStatStdXP          0x31
#define tStatLower          0x32
#define tStatUpper          0x33
#define tStat_s             0x34
#define tLRSqr              0x35
#define tBRSqr              0x36

// These next tokens are only used to access the data
// They are display only and the user cannot access them at all
#define tF_DF               0x37
#define tF_SS               0x38
#define tF_MS               0x39
#define tE_DF               0x3A
#define tE_SS               0x3B
#define tE_MS               0x3C

// 2nd Half Of System Input/Output Variables
#define tuXscl              0x00
#define tuYscl              0x01
#define tXscl               0x02
#define tYscl               0x03
#define tRecuru0            0x04
#define tRecurv0            0x05
#define tun1                0x06
#define tvn1                0x07
#define tuRecuru0           0x08
#define tuRecurv0           0x09
#define tXmin               0x0A
#define tXmax               0x0B
#define tYmin               0x0C
#define tYmax               0x0D
#define tTmin               0x0E
#define tTmax               0x0F
#define tThetaMin           0x10
#define tThetaMax           0x11
#define tuXmin              0x12
#define tuXmax              0x13
#define tuYmin              0x14
#define tuYmax              0x15
#define tuThetMin           0x16
#define tuThetMax           0x17
#define tuTmin              0x18
#define tuTmax              0x19
#define tTblMin             0x1A
#define tPlotStart          0x1B
#define tuPlotStart         0x1C
#define tnMax               0x1D
#define tunMax              0x1E
#define tnMin               0x1F
#define tunMin              0x20
#define tTblStep            0x21
#define tTStep              0x22
#define tThetaStep          0x23
#define tuTStep             0x24
#define tuThetStep          0x25
#define tDeltaX             0x26
#define tDeltaY             0x27
#define tXFact              0x28
#define tYFact              0x29
#define tTblInput           0x2A
#define tFinN               0x2B
#define tFinI               0x2C
#define tFinPV              0x2D
#define tFinPMT             0x2E
#define tFinFV              0x2F
#define tFinPY              0x30
#define tFinCY              0x31
#define tRecurw0            0x32
#define tuRecurw0           0x33
#define tPlotStep           0x34
#define tuPlotStep          0x35
#define tXres               0x36
#define tuXres              0x37
#define tRecuru02           0x38
#define tuRecuru02          0x39
#define tRecurv02           0x3C
#define tuRecurv02          0x3D
#define tRecurw02           0x3E
#define tuRecurw02          0x3F

// 2nd Byte Of t2ByteTok Tokens
#define tFinNPV             0x00
#define tFinIRR             0x01
#define tFinBAL             0x02
#define tFinPRN             0x03
#define tFinINT             0x04
#define tFinToNom           0x05
#define tFinToEff           0x06
#define tFinDBD             0x07
#define tLCM                0x08
#define tGCD                0x09
#define tRandInt            0x0A
#define tRandBin            0x0B
#define tSubStrng           0x0C
#define tStdDev             0x0D
#define tVariance           0x0E
#define tInStrng            0x0F
#define tDNormal            0x10
#define tInvNorm            0x11
#define tDT                 0x12
#define tChI                0x13
#define tDF                 0x14
#define tBINPDF             0x15
#define tBINCDF             0x16
#define tPOIPDF             0x17
#define tPOICDF             0x18
#define tGEOPDF             0x19
#define tGEOCDF             0x1A
#define tNormalPDF          0x1B
#define tTPDF               0x1C
#define tChiPDF             0x1D
#define tFPDF               0x1E
#define tRandNorm           0x1F
#define tFinFPMT            0x20
#define tFinFI              0x21
#define tFinFPV             0x22
#define tFinFN              0x23
#define tFinFFV             0x24
#define tConj               0x25
#define tReal               0x26
#define tImag               0x27
#define tAngle              0x28
#define tCumSum             0x29
#define tExpr               0x2A
#define tLength             0x2B
#define tDeltaLst           0x2C
#define tRef                0x2D
#define tRRef               0x2E
#define tToRect             0x2F
#define tToPolar            0x30
#define tConste             0x31
#define tSinReg             0x32
#define tLogistic           0x33
#define tLinRegTTest        0x34
#define tShadeNorm          0x35
#define tShadeT             0x36
#define tShadeChi           0x37
#define tShadeF             0x38
#define tMatToLst           0x39
#define tLstToMat           0x3A
#define tZTest              0x3B
#define tTTest              0x3C
#define t2SampZTest         0x3D
#define t1PropZTest         0x3E
#define t2PropZTest         0x3F
#define tChiTest            0x40
#define tZIntVal            0x41
#define t2SampZInt          0x42
#define t1PropZInt          0x43
#define t2PropZInt          0x44
#define tGraphStyle         0x45
#define t2SampTTest         0x46
#define t2SampFTest         0x47
#define tTIntVal            0x48
#define t2SampTInt          0x49
#define tSetupLst           0x4A
#define tFinPMTend          0x4B
#define tFinPMTbeg          0x4C
#define tRealM              0x4D
#define tPolarM             0x4E
#define tRectM              0x4F
#define tExprOn             0x50
#define tExprOff            0x51
#define tClrAllLst          0x52
#define tGetCalc            0x53
#define tDelVar             0x54
#define tEquToStrng         0x55
#define tStrngToequ         0x56
#define tDelLast            0x57
#define tSelect             0x58
#define tANOVA              0x59
#define tModBox             0x5A
#define tNormProb           0x5B
#define tMGT                0x64
#define tZFit               0x65
#define tDiag_on            0x66
#define tDiag_off           0x67
#define tArchive            0x68
#define tUnarchive          0x69
#define tAsm                0x6A
#define tAsmComp            0x6B
#define tAsmPrgm            0x6C
#define tAsmCmp             0x6D
#define tLcapAAcute         0x6E
#define tLcapAGrave         0x6F
#define tLcapACaret         0x70
#define tLcapADier          0x71
#define tLaAcute            0x72
#define tLaGrave            0x73
#define tLaCaret            0x74
#define tLaDier             0x75
#define tLcapEAcute         0x76
#define tLcapEGrave         0x77
#define tLcapECaret         0x78
#define tLcapEDier          0x79
#define tLeAcute            0x7A
#define tLeGrave            0x7B
#define tLeCaret            0x7C
#define tLeDier             0x7D
#define tLcapIGrave         0x7F
#define tLcapICaret         0x80
#define tLcapIDier          0x81
#define tLiAcute            0x82
#define tLiGrave            0x83
#define tLiCaret            0x84
#define tLiDier             0x85
#define tLcapOAcute         0x86
#define tLcapOGrave         0x87
#define tLcapOCaret         0x88
#define tLcapODier          0x89
#define tLoAcute            0x8A
#define tLoGrave            0x8B
#define tLoCaret            0x8C
#define tLoDier             0x8D
#define tLcapUAcute         0x8E
#define tLcapUGrave         0x8F
#define tLcapUCaret         0x90
#define tLcapUDier          0x91
#define tLuAcute            0x92
#define tLuGrave            0x93
#define tLuCaret            0x94
#define tLuDier             0x95
#define tLcapCCed           0x96
#define tLcCed              0x97
#define tLcapNTilde         0x98
#define tLnTilde            0x99
#define tLaccent            0x9A
#define tLgrave             0x9B
#define tLdieresis          0x9C
#define tLquesDown          0x9D
#define tLexclamDown        0x9E
#define tLalpha             0x9F
#define tLbeta              0xA0
#define tLgamma             0xA1
#define tLcapDelta          0xA2
#define tLdelta             0xA3
#define tLepsilon           0xA4
#define tLlambda            0xA5
#define tLmu                0xA6
#define tLpi                0xA7
#define tLrho               0xA8
#define tLcapSigma          0xA9
#define tLphi               0xAB
#define tLcapOmega          0xAC
#define tLphat              0xAD
#define tLchi               0xAE
#define tLstatF             0xAF
#define tLa                 0xB0
#define tLb                 0xB1
#define tLc                 0xB2
#define tLd                 0xB3
#define tLsmalle            0xB4
#define tLf                 0xB5
#define tLsmallg            0xB6
#define tLh                 0xB7
#define tLi                 0xB8
#define tLj                 0xB9
#define tLk                 0xBA
#define tLl                 0xBC
#define tLm                 0xBD
#define tLsmalln            0xBE
#define tLo                 0xBF
#define tLp                 0xC0
#define tLq                 0xC1
#define tLsmallr            0xC2
#define tLs                 0xC3
#define tLsmallt            0xC4
#define tLu                 0xC5
#define tLv                 0xC6
#define tLw                 0xC7
#define tLx                 0xC8
#define tLy                 0xC9
#define tLz                 0xCA
#define tLsigma             0xCB
#define tLtau               0xCC
#define tLcapIAcute         0xCD
#define tGarbageCollect     0xCE

/* 2 byte extended tokens (tExtTok) present in OS 5.2 and above */
#define tSEQn               0x8F /* 'SEQ(n)'     */
#define tSEQn1              0x90 /* 'SEQ(n+1)'   */
#define tSEQn2              0x91 /* 'SEQ(n+2)'   */
#define tLEFT               0x92 /* 'LEFT'       */
#define tCENTER             0x93 /* 'CENTER'     */
#define tRIGHT              0x94 /* 'RIGHT'      */
#define tInvBinom           0x95 /* 'invBinom('  */
#define tWait               0x96 /* 'Wait_'      */
#define tToString           0x97 /* 'toString('  */
#define tEval               0x98 /* 'eval('      */

/*
 * --- TIOS System error codes ---
 */
#define OS_E_EDIT           1<<7
#define OS_E_MASK           0x7F
#define OS_E_OVERFLOW       1+OS_E_EDIT
#define OS_E_DIVBY0         2+OS_E_EDIT
#define OS_E_SINGULARMAT    3+OS_E_EDIT
#define OS_E_DOMAIN         4+OS_E_EDIT
#define OS_E_INCREMENT      5+OS_E_EDIT
#define OS_E_BREAK          6+OS_E_EDIT
#define OS_E_NONREAL        7+OS_E_EDIT
#define OS_E_SYNTAX         8+OS_E_EDIT
#define OS_E_DATATYPE       9+OS_E_EDIT
#define OS_E_ARGUMENT       10+OS_E_EDIT
#define OS_E_DIMMISMATCH    11+OS_E_EDIT
#define OS_E_DIMENSION      12+OS_E_EDIT
#define OS_E_UNDEFINED      13+OS_E_EDIT
#define OS_E_MEMORY         14+OS_E_EDIT
#define OS_E_INVALID        15+OS_E_EDIT
#define OS_E_ILLEGALNEST    16+OS_E_EDIT
#define OS_E_BOUND          17+OS_E_EDIT
#define OS_E_GRAPHRANGE     18+OS_E_EDIT
#define OS_E_ZOOM           19+OS_E_EDIT
#define OS_E_LABEL          20
#define OS_E_STAT           21
#define OS_E_SOLVER         22+OS_E_EDIT
#define OS_E_SINGULARITY    23+OS_E_EDIT
#define OS_E_SIGNCHANGE     24+OS_E_EDIT
#define OS_E_ITERATIONS     25+OS_E_EDIT
#define OS_E_BADGUESS       26+OS_E_EDIT
#define OS_E_STATPLOT       27
#define OS_E_TOLTOOSMALL    28+OS_E_EDIT
#define OS_E_RESERVED       29+OS_E_EDIT
#define OS_E_MODE           30+OS_E_EDIT
#define OS_E_LNKERR         31+OS_E_EDIT
#define OS_E_LNKMEMERR      32+OS_E_EDIT
#define OS_E_LNKTRANSERR    33+OS_E_EDIT
#define OS_E_LNKDUPERR      34+OS_E_EDIT
#define OS_E_LNKMEMFULL     35+OS_E_EDIT
#define OS_E_UNKNOWN        36+OS_E_EDIT
#define OS_E_SCALE          37+OS_E_EDIT
#define OS_E_IDNOTFOUND     38
#define OS_E_NOMODE         39+OS_E_EDIT
#define OS_E_VALIDATION     40
#define OS_E_LENGTH         41+OS_E_EDIT
#define OS_E_APPLICATION    42+OS_E_EDIT
#define OS_E_APPERR1        43+OS_E_EDIT
#define OS_E_APPERR2        44+OS_E_EDIT
#define OS_E_EXPIREDAPP     45
#define OS_E_BADADD         46
#define OS_E_ARCHIVED       47+OS_E_EDIT
#define OS_E_VERSION        48
#define OS_E_ARCHFULL       49
#define OS_E_VARIABLE       50+OS_E_EDIT
#define OS_E_DUPLICATE      51+OS_E_EDIT

/*
 * --- TI-OS os_GetCSC Scan Code Return Values ---
 */
#define sk_Down             0x01
#define sk_Left             0x02
#define sk_Right            0x03
#define sk_Up               0x04
#define sk_Enter            0x09
#define sk_2nd              0x36
#define sk_Clear            0x0F
#define sk_Alpha            0x30
#define sk_Add              0x0A
#define sk_Sub              0x0B
#define sk_Mul              0x0C
#define sk_Div              0x0D
#define sk_Graph            0x31
#define sk_Trace            0x32
#define sk_Zoom             0x33
#define sk_Window           0x34
#define sk_Yequ             0x35
#define sk_Mode             0x37
#define sk_Del              0x38
#define sk_Store            0x2A
#define sk_Ln               0x2B
#define sk_Log              0x2C
#define sk_Square           0x2D
#define sk_Recip            0x2E
#define sk_Math             0x2F
#define sk_0                0x21
#define sk_1                0x22
#define sk_4                0x23
#define sk_7                0x24
#define sk_2                0x1A
#define sk_5                0x1B
#define sk_8                0x1C
#define sk_3                0x12
#define sk_6                0x13
#define sk_9                0x14
#define sk_Comma            0x25
#define sk_Sin              0x26
#define sk_Apps             0x27
#define sk_GraphVar         0x28
#define sk_DecPnt           0x19
#define sk_LParen           0x1D
#define sk_Cos              0x1E
#define sk_Prgm             0x1F
#define sk_Stat             0x20
#define sk_Chs              0x10
#define sk_RParen           0x15
#define sk_Tan              0x16
#define sk_Vars             0x17
#define sk_Power            0x0E

#ifdef __cplusplus
}
#endif

#endif
