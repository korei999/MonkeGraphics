#pragma once
#include "adt/mathDecl.hh"

/* https://www.rapidtables.com/web/color/RGB_Color.html */

namespace colors
{

enum class IDX : int
{
    MAROON,
    DARKRED,
    BROWN,
    FIREBRICK,
    CRIMSON,
    RED,
    TOMATO,
    CORAL,
    INDIANRED,
    LIGHTCORAL,
    DARKSALMON,
    SALMON,
    LIGHTSALMON,
    ORANGERED,
    DARKORANGE,
    ORANGE,
    GOLD,
    DARKGOLDENROD,
    GOLDENROD,
    PALEGOLDENROD,
    DARKKHAKI,
    KHAKI,
    OLIVE,
    YELLOW,
    YELLOWGREEN,
    DARKOLIVEGREEN,
    OLIVEDRAB,
    LAWNGREEN,
    CHARTREUSE,
    GREENYELLOW,
    DARKGREEN,
    GREEN,
    FORESTGREEN,
    LIME,
    LIMEGREEN,
    LIGHTGREEN,
    PALEGREEN,
    DARKSEAGREEN,
    MEDIUMSPRINGGREEN,
    SPRINGGREEN,
    SEAGREEN,
    MEDIUMAQUAMARINE,
    MEDIUMSEAGREEN,
    LIGHTSEAGREEN,
    DARKSLATEGRAY,
    TEAL,
    DARKCYAN,
    AQUA,
    CYAN,
    LIGHTCYAN,
    DARKTURQUOISE,
    TURQUOISE,
    MEDIUMTURQUOISE,
    PALETURQUOISE,
    AQUAMARINE,
    POWDERBLUE,
    CADETBLUE,
    STEELBLUE,
    CORNFLOWERBLUE,
    DEEPSKYBLUE,
    DODGERBLUE,
    LIGHTBLUE,
    SKYBLUE,
    LIGHTSKYBLUE,
    MIDNIGHTBLUE,
    NAVY,
    DARKBLUE,
    MEDIUMBLUE,
    BLUE,
    ROYALBLUE,
    BLUEVIOLET,
    INDIGO,
    DARKSLATEBLUE,
    SLATEBLUE,
    MEDIUMSLATEBLUE,
    MEDIUMPURPLE,
    DARKMAGENTA,
    DARKVIOLET,
    DARKORCHID,
    MEDIUMORCHID,
    PURPLE,
    THISTLE,
    PLUM,
    VIOLET,
    MAGENTA,
    ORCHID,
    MEDIUMVIOLETRED,
    PALEVIOLETRED,
    DEEPPINK,
    HOTPINK,
    LIGHTPINK,
    PINK,
    ANTIQUEWHITE,
    BEIGE,
    BISQUE,
    BLANCHEDALMOND,
    WHEAT,
    CORNSILK,
    LEMONCHIFFON,
    LIGHTGOLDENRODYELLOW,
    LIGHTYELLOW,
    SADDLEBROWN,
    SIENNA,
    CHOCOLATE,
    PERU,
    SANDYBROWN,
    BURLYWOOD,
    TAN,
    ROSYBROWN,
    MOCCASIN,
    NAVAJOWHITE,
    PEACHPUFF,
    MISTYROSE,
    LAVENDERBLUSH,
    LINEN,
    OLDLACE,
    PAPAYAWHIP,
    SEASHELL,
    MINTCREAM,
    SLATEGRAY,
    LIGHTSLATEGRAY,
    LIGHTSTEELBLUE,
    LAVENDER,
    FLORALWHITE,
    ALICEBLUE,
    GHOSTWHITE,
    HONEYDEW,
    IVORY,
    AZURE,
    SNOW,
    BLACK,
    DIMGREY,
    GREY,
    DARKGREY,
    SILVER,
    LIGHTGREY,
    GAINSBORO,
    WHITESMOKE,
    WHITE,
    ESIZE
};

constexpr auto MAROON = adt::math::V3{0.501961f, 0.0f, 0.0f}; /* MAROON */
constexpr auto DARKRED = adt::math::V3{0.545098f, 0.0f, 0.0f}; /* DARKRED */
constexpr auto BROWN = adt::math::V3{0.647059f, 0.164706f, 0.164706f}; /* BROWN */
constexpr auto FIREBRICK = adt::math::V3{0.698039f, 0.133333f, 0.133333f}; /* FIREBRICK */
constexpr auto CRIMSON = adt::math::V3{0.862745f, 0.0784314f, 0.235294f}; /* CRIMSON */
constexpr auto RED = adt::math::V3{1.0f, 0.0f, 0.0f}; /* RED */
constexpr auto TOMATO = adt::math::V3{1.0f, 0.388235f, 0.278431f}; /* TOMATO */
constexpr auto CORAL = adt::math::V3{1.0f, 0.498039f, 0.313726f}; /* CORAL */
constexpr auto INDIANRED = adt::math::V3{0.803922f, 0.360784f, 0.360784f}; /* INDIANRED */
constexpr auto LIGHTCORAL = adt::math::V3{0.941176f, 0.501961f, 0.501961f}; /* LIGHTCORAL */
constexpr auto DARKSALMON = adt::math::V3{0.913725f, 0.588235f, 0.478431f}; /* DARKSALMON */
constexpr auto SALMON = adt::math::V3{0.980392f, 0.501961f, 0.447059f}; /* SALMON */
constexpr auto LIGHTSALMON = adt::math::V3{1.0f, 0.627451f, 0.478431f}; /* LIGHTSALMON */
constexpr auto ORANGERED = adt::math::V3{1.0f, 0.270588f, 0.0f}; /* ORANGERED */
constexpr auto DARKORANGE = adt::math::V3{1.0f, 0.54902f, 0.0f}; /* DARKORANGE */
constexpr auto ORANGE = adt::math::V3{1.0f, 0.647059f, 0.0f}; /* ORANGE */
constexpr auto GOLD = adt::math::V3{1.0f, 0.843137f, 0.0f}; /* GOLD */
constexpr auto DARKGOLDENROD = adt::math::V3{0.721569f, 0.52549f, 0.0431373f}; /* DARKGOLDENROD */
constexpr auto GOLDENROD = adt::math::V3{0.854902f, 0.647059f, 0.12549f}; /* GOLDENROD */
constexpr auto PALEGOLDENROD = adt::math::V3{0.933333f, 0.909804f, 0.666667f}; /* PALEGOLDENROD */
constexpr auto DARKKHAKI = adt::math::V3{0.741176f, 0.717647f, 0.419608f}; /* DARKKHAKI */
constexpr auto KHAKI = adt::math::V3{0.941176f, 0.901961f, 0.54902f}; /* KHAKI */
constexpr auto OLIVE = adt::math::V3{0.501961f, 0.501961f, 0.0f}; /* OLIVE */
constexpr auto YELLOW = adt::math::V3{1.0f, 1.0f, 0.0f}; /* YELLOW */
constexpr auto YELLOWGREEN = adt::math::V3{0.603922f, 0.803922f, 0.196078f}; /* YELLOWGREEN */
constexpr auto DARKOLIVEGREEN = adt::math::V3{0.333333f, 0.419608f, 0.184314f}; /* DARKOLIVEGREEN */
constexpr auto OLIVEDRAB = adt::math::V3{0.419608f, 0.556863f, 0.137255f}; /* OLIVEDRAB */
constexpr auto LAWNGREEN = adt::math::V3{0.486275f, 0.988235f, 0.0f}; /* LAWNGREEN */
constexpr auto CHARTREUSE = adt::math::V3{0.498039f, 1.0f, 0.0f}; /* CHARTREUSE */
constexpr auto GREENYELLOW = adt::math::V3{0.678431f, 1.0f, 0.184314f}; /* GREENYELLOW */
constexpr auto DARKGREEN = adt::math::V3{0.0f, 0.392157f, 0.0f}; /* DARKGREEN */
constexpr auto GREEN = adt::math::V3{0.0f, 0.501961f, 0.0f}; /* GREEN */
constexpr auto FORESTGREEN = adt::math::V3{0.133333f, 0.545098f, 0.133333f}; /* FORESTGREEN */
constexpr auto LIME = adt::math::V3{0.0f, 1.0f, 0.0f}; /* LIME */
constexpr auto LIMEGREEN = adt::math::V3{0.196078f, 0.803922f, 0.196078f}; /* LIMEGREEN */
constexpr auto LIGHTGREEN = adt::math::V3{0.564706f, 0.933333f, 0.564706f}; /* LIGHTGREEN */
constexpr auto PALEGREEN = adt::math::V3{0.596078f, 0.984314f, 0.596078f}; /* PALEGREEN */
constexpr auto DARKSEAGREEN = adt::math::V3{0.560784f, 0.737255f, 0.560784f}; /* DARKSEAGREEN */
constexpr auto MEDIUMSPRINGGREEN = adt::math::V3{0.0f, 0.980392f, 0.603922f}; /* MEDIUMSPRINGGREEN */
constexpr auto SPRINGGREEN = adt::math::V3{0.0f, 1.0f, 0.498039f}; /* SPRINGGREEN */
constexpr auto SEAGREEN = adt::math::V3{0.180392f, 0.545098f, 0.341176f}; /* SEAGREEN */
constexpr auto MEDIUMAQUAMARINE = adt::math::V3{0.4f, 0.803922f, 0.666667f}; /* MEDIUMAQUAMARINE */
constexpr auto MEDIUMSEAGREEN = adt::math::V3{0.235294f, 0.701961f, 0.443137f}; /* MEDIUMSEAGREEN */
constexpr auto LIGHTSEAGREEN = adt::math::V3{0.12549f, 0.698039f, 0.666667f}; /* LIGHTSEAGREEN */
constexpr auto DARKSLATEGRAY = adt::math::V3{0.184314f, 0.309804f, 0.309804f}; /* DARKSLATEGRAY */
constexpr auto TEAL = adt::math::V3{0.0f, 0.501961f, 0.501961f}; /* TEAL */
constexpr auto DARKCYAN = adt::math::V3{0.0f, 0.545098f, 0.545098f}; /* DARKCYAN */
constexpr auto AQUA = adt::math::V3{0.0f, 1.0f, 1.0f}; /* AQUA */
constexpr auto CYAN = adt::math::V3{0.0f, 1.0f, 1.0f}; /* CYAN */
constexpr auto LIGHTCYAN = adt::math::V3{0.878431f, 1.0f, 1.0f}; /* LIGHTCYAN */
constexpr auto DARKTURQUOISE = adt::math::V3{0.0f, 0.807843f, 0.819608f}; /* DARKTURQUOISE */
constexpr auto TURQUOISE = adt::math::V3{0.25098f, 0.878431f, 0.815686f}; /* TURQUOISE */
constexpr auto MEDIUMTURQUOISE = adt::math::V3{0.282353f, 0.819608f, 0.8f}; /* MEDIUMTURQUOISE */
constexpr auto PALETURQUOISE = adt::math::V3{0.686275f, 0.933333f, 0.933333f}; /* PALETURQUOISE */
constexpr auto AQUAMARINE = adt::math::V3{0.498039f, 1.0f, 0.831373f}; /* AQUAMARINE */
constexpr auto POWDERBLUE = adt::math::V3{0.690196f, 0.878431f, 0.901961f}; /* POWDERBLUE */
constexpr auto CADETBLUE = adt::math::V3{0.372549f, 0.619608f, 0.627451f}; /* CADETBLUE */
constexpr auto STEELBLUE = adt::math::V3{0.27451f, 0.509804f, 0.705882f}; /* STEELBLUE */
constexpr auto CORNFLOWERBLUE = adt::math::V3{0.392157f, 0.584314f, 0.929412f}; /* CORNFLOWERBLUE */
constexpr auto DEEPSKYBLUE = adt::math::V3{0.0f, 0.74902f, 1.0f}; /* DEEPSKYBLUE */
constexpr auto DODGERBLUE = adt::math::V3{0.117647f, 0.564706f, 1.0f}; /* DODGERBLUE */
constexpr auto LIGHTBLUE = adt::math::V3{0.678431f, 0.847059f, 0.901961f}; /* LIGHTBLUE */
constexpr auto SKYBLUE = adt::math::V3{0.529412f, 0.807843f, 0.921569f}; /* SKYBLUE */
constexpr auto LIGHTSKYBLUE = adt::math::V3{0.529412f, 0.807843f, 0.980392f}; /* LIGHTSKYBLUE */
constexpr auto MIDNIGHTBLUE = adt::math::V3{0.0980392f, 0.0980392f, 0.439216f}; /* MIDNIGHTBLUE */
constexpr auto NAVY = adt::math::V3{0.0f, 0.0f, 0.501961f}; /* NAVY */
constexpr auto DARKBLUE = adt::math::V3{0.0f, 0.0f, 0.545098f}; /* DARKBLUE */
constexpr auto MEDIUMBLUE = adt::math::V3{0.0f, 0.0f, 0.803922f}; /* MEDIUMBLUE */
constexpr auto BLUE = adt::math::V3{0.0f, 0.0f, 1.0f}; /* BLUE */
constexpr auto ROYALBLUE = adt::math::V3{0.254902f, 0.411765f, 0.882353f}; /* ROYALBLUE */
constexpr auto BLUEVIOLET = adt::math::V3{0.541176f, 0.168627f, 0.886275f}; /* BLUEVIOLET */
constexpr auto INDIGO = adt::math::V3{0.294118f, 0.0f, 0.509804f}; /* INDIGO */
constexpr auto DARKSLATEBLUE = adt::math::V3{0.282353f, 0.239216f, 0.545098f}; /* DARKSLATEBLUE */
constexpr auto SLATEBLUE = adt::math::V3{0.415686f, 0.352941f, 0.803922f}; /* SLATEBLUE */
constexpr auto MEDIUMSLATEBLUE = adt::math::V3{0.482353f, 0.407843f, 0.933333f}; /* MEDIUMSLATEBLUE */
constexpr auto MEDIUMPURPLE = adt::math::V3{0.576471f, 0.439216f, 0.858824f}; /* MEDIUMPURPLE */
constexpr auto DARKMAGENTA = adt::math::V3{0.545098f, 0.0f, 0.545098f}; /* DARKMAGENTA */
constexpr auto DARKVIOLET = adt::math::V3{0.580392f, 0.0f, 0.827451f}; /* DARKVIOLET */
constexpr auto DARKORCHID = adt::math::V3{0.6f, 0.196078f, 0.8f}; /* DARKORCHID */
constexpr auto MEDIUMORCHID = adt::math::V3{0.729412f, 0.333333f, 0.827451f}; /* MEDIUMORCHID */
constexpr auto PURPLE = adt::math::V3{0.501961f, 0.0f, 0.501961f}; /* PURPLE */
constexpr auto THISTLE = adt::math::V3{0.847059f, 0.74902f, 0.847059f}; /* THISTLE */
constexpr auto PLUM = adt::math::V3{0.866667f, 0.627451f, 0.866667f}; /* PLUM */
constexpr auto VIOLET = adt::math::V3{0.933333f, 0.509804f, 0.933333f}; /* VIOLET */
constexpr auto MAGENTA = adt::math::V3{1.0f, 0.0f, 1.0f}; /* MAGENTA */
constexpr auto ORCHID = adt::math::V3{0.854902f, 0.439216f, 0.839216f}; /* ORCHID */
constexpr auto MEDIUMVIOLETRED = adt::math::V3{0.780392f, 0.0823529f, 0.521569f}; /* MEDIUMVIOLETRED */
constexpr auto PALEVIOLETRED = adt::math::V3{0.858824f, 0.439216f, 0.576471f}; /* PALEVIOLETRED */
constexpr auto DEEPPINK = adt::math::V3{1.0f, 0.0784314f, 0.576471f}; /* DEEPPINK */
constexpr auto HOTPINK = adt::math::V3{1.0f, 0.411765f, 0.705882f}; /* HOTPINK */
constexpr auto LIGHTPINK = adt::math::V3{1.0f, 0.713726f, 0.756863f}; /* LIGHTPINK */
constexpr auto PINK = adt::math::V3{1.0f, 0.752941f, 0.796078f}; /* PINK */
constexpr auto ANTIQUEWHITE = adt::math::V3{0.980392f, 0.921569f, 0.843137f}; /* ANTIQUEWHITE */
constexpr auto BEIGE = adt::math::V3{0.960784f, 0.960784f, 0.862745f}; /* BEIGE */
constexpr auto BISQUE = adt::math::V3{1.0f, 0.894118f, 0.768627f}; /* BISQUE */
constexpr auto BLANCHEDALMOND = adt::math::V3{1.0f, 0.921569f, 0.803922f}; /* BLANCHEDALMOND */
constexpr auto WHEAT = adt::math::V3{0.960784f, 0.870588f, 0.701961f}; /* WHEAT */
constexpr auto CORNSILK = adt::math::V3{1.0f, 0.972549f, 0.862745f}; /* CORNSILK */
constexpr auto LEMONCHIFFON = adt::math::V3{1.0f, 0.980392f, 0.803922f}; /* LEMONCHIFFON */
constexpr auto LIGHTGOLDENRODYELLOW = adt::math::V3{0.980392f, 0.980392f, 0.823529f}; /* LIGHTGOLDENRODYELLOW */
constexpr auto LIGHTYELLOW = adt::math::V3{1.0f, 1.0f, 0.878431f}; /* LIGHTYELLOW */
constexpr auto SADDLEBROWN = adt::math::V3{0.545098f, 0.270588f, 0.0745098f}; /* SADDLEBROWN */
constexpr auto SIENNA = adt::math::V3{0.627451f, 0.321569f, 0.176471f}; /* SIENNA */
constexpr auto CHOCOLATE = adt::math::V3{0.823529f, 0.411765f, 0.117647f}; /* CHOCOLATE */
constexpr auto PERU = adt::math::V3{0.803922f, 0.521569f, 0.247059f}; /* PERU */
constexpr auto SANDYBROWN = adt::math::V3{0.956863f, 0.643137f, 0.376471f}; /* SANDYBROWN */
constexpr auto BURLYWOOD = adt::math::V3{0.870588f, 0.721569f, 0.529412f}; /* BURLYWOOD */
constexpr auto TAN = adt::math::V3{0.823529f, 0.705882f, 0.54902f}; /* TAN */
constexpr auto ROSYBROWN = adt::math::V3{0.737255f, 0.560784f, 0.560784f}; /* ROSYBROWN */
constexpr auto MOCCASIN = adt::math::V3{1.0f, 0.894118f, 0.709804f}; /* MOCCASIN */
constexpr auto NAVAJOWHITE = adt::math::V3{1.0f, 0.870588f, 0.678431f}; /* NAVAJOWHITE */
constexpr auto PEACHPUFF = adt::math::V3{1.0f, 0.854902f, 0.72549f}; /* PEACHPUFF */
constexpr auto MISTYROSE = adt::math::V3{1.0f, 0.894118f, 0.882353f}; /* MISTYROSE */
constexpr auto LAVENDERBLUSH = adt::math::V3{1.0f, 0.941176f, 0.960784f}; /* LAVENDERBLUSH */
constexpr auto LINEN = adt::math::V3{0.980392f, 0.941176f, 0.901961f}; /* LINEN */
constexpr auto OLDLACE = adt::math::V3{0.992157f, 0.960784f, 0.901961f}; /* OLDLACE */
constexpr auto PAPAYAWHIP = adt::math::V3{1.0f, 0.937255f, 0.835294f}; /* PAPAYAWHIP */
constexpr auto SEASHELL = adt::math::V3{1.0f, 0.960784f, 0.933333f}; /* SEASHELL */
constexpr auto MINTCREAM = adt::math::V3{0.960784f, 1.0f, 0.980392f}; /* MINTCREAM */
constexpr auto SLATEGRAY = adt::math::V3{0.439216f, 0.501961f, 0.564706f}; /* SLATEGRAY */
constexpr auto LIGHTSLATEGRAY = adt::math::V3{0.466667f, 0.533333f, 0.6f}; /* LIGHTSLATEGRAY */
constexpr auto LIGHTSTEELBLUE = adt::math::V3{0.690196f, 0.768627f, 0.870588f}; /* LIGHTSTEELBLUE */
constexpr auto LAVENDER = adt::math::V3{0.901961f, 0.901961f, 0.980392f}; /* LAVENDER */
constexpr auto FLORALWHITE = adt::math::V3{1.0f, 0.980392f, 0.941176f}; /* FLORALWHITE */
constexpr auto ALICEBLUE = adt::math::V3{0.941176f, 0.972549f, 1.0f}; /* ALICEBLUE */
constexpr auto GHOSTWHITE = adt::math::V3{0.972549f, 0.972549f, 1.0f}; /* GHOSTWHITE */
constexpr auto HONEYDEW = adt::math::V3{0.941176f, 1.0f, 0.941176f}; /* HONEYDEW */
constexpr auto IVORY = adt::math::V3{1.0f, 1.0f, 0.941176f}; /* IVORY */
constexpr auto AZURE = adt::math::V3{0.941176f, 1.0f, 1.0f}; /* AZURE */
constexpr auto SNOW = adt::math::V3{1.0f, 0.980392f, 0.980392f}; /* SNOW */
constexpr auto BLACK = adt::math::V3{0.0f, 0.0f, 0.0f}; /* BLACK */
constexpr auto DIMGREY = adt::math::V3{0.411765f, 0.411765f, 0.411765f}; /* DIMGREY */
constexpr auto GREY = adt::math::V3{0.501961f, 0.501961f, 0.501961f}; /* GREY */
constexpr auto DARKGREY = adt::math::V3{0.662745f, 0.662745f, 0.662745f}; /* DARKGREY */
constexpr auto SILVER = adt::math::V3{0.752941f, 0.752941f, 0.752941f}; /* SILVER */
constexpr auto LIGHTGREY = adt::math::V3{0.827451f, 0.827451f, 0.827451f}; /* LIGHTGREY */
constexpr auto GAINSBORO = adt::math::V3{0.862745f, 0.862745f, 0.862745f}; /* GAINSBORO */
constexpr auto WHITESMOKE = adt::math::V3{0.960784f, 0.960784f, 0.960784f}; /* WHITESMOKE */
constexpr auto WHITE = adt::math::V3{1.0f, 1.0f, 1.0f}; /* WHITE */

constexpr adt::math::V3 map[] {
    MAROON,
    DARKRED,
    BROWN,
    FIREBRICK,
    CRIMSON,
    RED,
    TOMATO,
    CORAL,
    INDIANRED,
    LIGHTCORAL,
    DARKSALMON,
    SALMON,
    LIGHTSALMON,
    ORANGERED,
    DARKORANGE,
    ORANGE,
    GOLD,
    DARKGOLDENROD,
    GOLDENROD,
    PALEGOLDENROD,
    DARKKHAKI,
    KHAKI,
    OLIVE,
    YELLOW,
    YELLOWGREEN,
    DARKOLIVEGREEN,
    OLIVEDRAB,
    LAWNGREEN,
    CHARTREUSE,
    GREENYELLOW,
    DARKGREEN,
    GREEN,
    FORESTGREEN,
    LIME,
    LIMEGREEN,
    LIGHTGREEN,
    PALEGREEN,
    DARKSEAGREEN,
    MEDIUMSPRINGGREEN,
    SPRINGGREEN,
    SEAGREEN,
    MEDIUMAQUAMARINE,
    MEDIUMSEAGREEN,
    LIGHTSEAGREEN,
    DARKSLATEGRAY,
    TEAL,
    DARKCYAN,
    AQUA,
    CYAN,
    LIGHTCYAN,
    DARKTURQUOISE,
    TURQUOISE,
    MEDIUMTURQUOISE,
    PALETURQUOISE,
    AQUAMARINE,
    POWDERBLUE,
    CADETBLUE,
    STEELBLUE,
    CORNFLOWERBLUE,
    DEEPSKYBLUE,
    DODGERBLUE,
    LIGHTBLUE,
    SKYBLUE,
    LIGHTSKYBLUE,
    MIDNIGHTBLUE,
    NAVY,
    DARKBLUE,
    MEDIUMBLUE,
    BLUE,
    ROYALBLUE,
    BLUEVIOLET,
    INDIGO,
    DARKSLATEBLUE,
    SLATEBLUE,
    MEDIUMSLATEBLUE,
    MEDIUMPURPLE,
    DARKMAGENTA,
    DARKVIOLET,
    DARKORCHID,
    MEDIUMORCHID,
    PURPLE,
    THISTLE,
    PLUM,
    VIOLET,
    MAGENTA,
    ORCHID,
    MEDIUMVIOLETRED,
    PALEVIOLETRED,
    DEEPPINK,
    HOTPINK,
    LIGHTPINK,
    PINK,
    ANTIQUEWHITE,
    BEIGE,
    BISQUE,
    BLANCHEDALMOND,
    WHEAT,
    CORNSILK,
    LEMONCHIFFON,
    LIGHTGOLDENRODYELLOW,
    LIGHTYELLOW,
    SADDLEBROWN,
    SIENNA,
    CHOCOLATE,
    PERU,
    SANDYBROWN,
    BURLYWOOD,
    TAN,
    ROSYBROWN,
    MOCCASIN,
    NAVAJOWHITE,
    PEACHPUFF,
    MISTYROSE,
    LAVENDERBLUSH,
    LINEN,
    OLDLACE,
    PAPAYAWHIP,
    SEASHELL,
    MINTCREAM,
    SLATEGRAY,
    LIGHTSLATEGRAY,
    LIGHTSTEELBLUE,
    LAVENDER,
    FLORALWHITE,
    ALICEBLUE,
    GHOSTWHITE,
    HONEYDEW,
    IVORY,
    AZURE,
    SNOW,
    BLACK,
    DIMGREY,
    GREY,
    DARKGREY,
    SILVER,
    LIGHTGREY,
    GAINSBORO,
    WHITESMOKE,
    WHITE,
};

inline constexpr adt::math::V3 get(IDX e) { return map[int(e)]; }

inline constexpr adt::math::V4
hexToV4(int hex)
{
    adt::f32 f = 1.0f / 255.0f;
    return {
        ((hex >> 24) & 0xff) * f,
            ((hex >> 16) & 0xff) * f,
            ((hex >> 8 ) & 0xff) * f,
            ((hex)       & 0xff) * f
    };
}

inline constexpr adt::math::V3
hexToV3(int hex)
{
    adt::f32 f = 1.0f / 255.0f;
    return {
        ((hex >> 16) & 0xff) * f,
            ((hex >> 8 ) & 0xff) * f,
            ((hex)       & 0xff) * f
    };
}

inline constexpr adt::u32
V4ToARGB(adt::math::V4 v)
{
    adt::u32 r {};
    r |= (adt::u32(v.r * 255.0f) << 8 * 3);
    r |= (adt::u32(v.g * 255.0f) << 8 * 2);
    r |= (adt::u32(v.b * 255.0f) << 8 * 1);
    r |=  adt::u32(v.a * 255.0f);

    return r;
}

inline constexpr adt::u32
V4ToRGBA(adt::math::V4 v)
{
    adt::u32 r {};
    r |= (adt::u32(v.r * 255.0f) << 8 * 0);
    r |= (adt::u32(v.g * 255.0f) << 8 * 1);
    r |= (adt::u32(v.b * 255.0f) << 8 * 2);
    r |=  adt::u32(v.a * 255.0f) << 8 * 3;

    return r;
}

inline constexpr adt::u32
V3ToHex(adt::math::V3 v)
{
    adt::u32 r {};
    r |= (adt::u32(v.x * 255.0f) << 8 * 2);
    r |= (adt::u32(v.y * 255.0f) << 8 * 1);
    r |=  adt::u32(v.z * 255.0f);

    return r;
}

} /* namespace colors */
