#include "msg_term_color.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static const char* color_to_ansi(int color_code) {
    switch (color_code) {
        case 0:   return "\033[0m";
        case 1:   return "\033[1m";
        case 2:   return "\033[2m";
        case 3:   return "\033[3m";
        case 4:   return "\033[4m";
        case 5:   return "\033[5m";
        case 7:   return "\033[7m";
        case 8:   return "\033[8m";
        case 9:   return "\033[9m";
        case 22:  return "\033[22m";
        case 23:  return "\033[23m";
        case 24:  return "\033[24m";
	case 25:  return "\033[25m";
        case 27:  return "\033[27m";
        case 28:  return "\033[28m";
        case 29:  return "\033[29m";
        case 30:  return "\033[30m";
        case 31:  return "\033[31m";
        case 32:  return "\033[32m";
        case 33:  return "\033[33m";
        case 34:  return "\033[34m";
        case 35:  return "\033[35m";
        case 36:  return "\033[36m";
        case 37:  return "\033[37m";
        case 40:  return "\033[40m";
        case 41:  return "\033[41m";
        case 42:  return "\033[42m";
        case 43:  return "\033[43m";
        case 44:  return "\033[44m";
        case 45:  return "\033[45m";
        case 46:  return "\033[46m";
        case 47:  return "\033[47m";
        case 90:  return "\033[90m";
        case 91:  return "\033[91m";
        case 92:  return "\033[92m";
        case 93:  return "\033[93m";
        case 94:  return "\033[94m";
        case 95:  return "\033[95m";
        case 96:  return "\033[96m";
        case 97:  return "\033[97m";
        case 100: return "\033[100m";
        case 101: return "\033[101m";
        case 102: return "\033[102m";
        case 103: return "\033[103m";
        case 104: return "\033[104m";
        case 105: return "\033[105m";
        case 106: return "\033[106m";
        case 107: return "\033[107m";
        case 5001: return "\033[38;5;1m";
        case 5002: return "\033[38;5;2m";
        case 5003: return "\033[38;5;3m";
        case 5004: return "\033[38;5;4m";
        case 5005: return "\033[38;5;5m";
        case 5006: return "\033[38;5;6m";
        case 5007: return "\033[38;5;7m";
        case 5008: return "\033[38;5;8m";
        case 5009: return "\033[38;5;9m";
        case 5010: return "\033[38;5;10m";
        case 5011: return "\033[38;5;11m";
        case 5012: return "\033[38;5;12m";
        case 5013: return "\033[38;5;13m";
        case 5014: return "\033[38;5;14m";
        case 5015: return "\033[38;5;15m";
        case 5016: return "\033[38;5;16m";
        case 5017: return "\033[38;5;17m";
        case 5018: return "\033[38;5;18m";
        case 5019: return "\033[38;5;19m";
        case 5020: return "\033[38;5;20m";
        case 5021: return "\033[38;5;21m";
        case 5022: return "\033[38;5;22m";
        case 5023: return "\033[38;5;23m";
        case 5024: return "\033[38;5;24m";
        case 5025: return "\033[38;5;25m";
        case 5026: return "\033[38;5;26m";
        case 5027: return "\033[38;5;27m";
        case 5028: return "\033[38;5;28m";
        case 5029: return "\033[38;5;29m";
        case 5030: return "\033[38;5;30m";
        case 5031: return "\033[38;5;31m";
        case 5032: return "\033[38;5;32m";
        case 5033: return "\033[38;5;33m";
        case 5034: return "\033[38;5;34m";
        case 5035: return "\033[38;5;35m";
        case 5036: return "\033[38;5;36m";
        case 5037: return "\033[38;5;37m";
        case 5038: return "\033[38;5;38m";
        case 5039: return "\033[38;5;39m";
        case 5040: return "\033[38;5;40m";
        case 5041: return "\033[38;5;41m";
        case 5042: return "\033[38;5;42m";
        case 5043: return "\033[38;5;43m";
        case 5044: return "\033[38;5;44m";
        case 5045: return "\033[38;5;45m";
        case 5046: return "\033[38;5;46m";
        case 5047: return "\033[38;5;47m";
        case 5048: return "\033[38;5;48m";
        case 5049: return "\033[38;5;49m";
        case 5050: return "\033[38;5;50m";
        case 5051: return "\033[38;5;51m";
        case 5052: return "\033[38;5;52m";
        case 5053: return "\033[38;5;53m";
        case 5054: return "\033[38;5;54m";
        case 5055: return "\033[38;5;55m";
        case 5056: return "\033[38;5;56m";
        case 5057: return "\033[38;5;56m";
        case 5058: return "\033[38;5;58m";
        case 5059: return "\033[38;5;59m";
        case 5060: return "\033[38;5;60m";
        case 5061: return "\033[38;5;61m";
        case 5062: return "\033[38;5;62m";
        case 5063: return "\033[38;5;63m";
        case 5064: return "\033[38;5;64m";
        case 5065: return "\033[38;5;65m";
        case 5066: return "\033[38;5;66m";
        case 5067: return "\033[38;5;67m";
        case 5068: return "\033[38;5;68m";
        case 5069: return "\033[38;5;69m";
        case 5070: return "\033[38;5;70m";
        case 5071: return "\033[38;5;71m";
        case 5072: return "\033[38;5;72m";
        case 5073: return "\033[38;5;73m";
        case 5074: return "\033[38;5;74m";
        case 5075: return "\033[38;5;75m";
        case 5076: return "\033[38;5;76m";
        case 5077: return "\033[38;5;77m";
        case 5078: return "\033[38;5;78m";
        case 5079: return "\033[38;5;79m";
        case 5080: return "\033[38;5;80m";
        case 5081: return "\033[38;5;81m";
        case 5082: return "\033[38;5;82m";
        case 5083: return "\033[38;5;83m";
        case 5084: return "\033[38;5;84m";
        case 5085: return "\033[38;5;85m";
        case 5086: return "\033[38;5;86m";
        case 5087: return "\033[38;5;87m";
        case 5088: return "\033[38;5;88m";
        case 5089: return "\033[38;5;89m";
        case 5090: return "\033[38;5;90m";
        case 5091: return "\033[38;5;91m";
        case 5092: return "\033[38;5;92m";
        case 5093: return "\033[38;5;93m";
        case 5094: return "\033[38;5;94m";
        case 5095: return "\033[38;5;95m";
        case 5096: return "\033[38;5;96m";
        case 5097: return "\033[38;5;97m";
        case 5098: return "\033[38;5;98m";
        case 5099: return "\033[38;5;99m";
        case 5100: return "\033[38;5;100m";
        case 5101: return "\033[38;5;101m";
        case 5102: return "\033[38;5;102m";
        case 5103: return "\033[38;5;103m";
        case 5104: return "\033[38;5;104m";
        case 5105: return "\033[38;5;105m";
        case 5106: return "\033[38;5;106m";
        case 5107: return "\033[38;5;107m";
        case 5108: return "\033[38;5;108m";
        case 5109: return "\033[38;5;109m";
        case 5110: return "\033[38;5;110m";
        case 5111: return "\033[38;5;111m";
        case 5112: return "\033[38;5;112m";
        case 5113: return "\033[38;5;113m";
        case 5114: return "\033[38;5;114m";
        case 5115: return "\033[38;5;115m";
        case 5116: return "\033[38;5;116m";
        case 5117: return "\033[38;5;117m";
        case 5118: return "\033[38;5;118m";
        case 5119: return "\033[38;5;119m";
        case 5120: return "\033[38;5;120m";
        case 5121: return "\033[38;5;121m";
        case 5122: return "\033[38;5;122m";
        case 5123: return "\033[38;5;123m";
        case 5124: return "\033[38;5;124m";
        case 5125: return "\033[38;5;125m";
        case 5126: return "\033[38;5;126m";
        case 5127: return "\033[38;5;127m";
        case 5128: return "\033[38;5;128m";
        case 5129: return "\033[38;5;129m";
        case 5130: return "\033[38;5;130m";
        case 5131: return "\033[38;5;131m";
        case 5132: return "\033[38;5;132m";
        case 5133: return "\033[38;5;133m";
        case 5134: return "\033[38;5;134m";
        case 5135: return "\033[38;5;135m";
        case 5136: return "\033[38;5;136m";
        case 5137: return "\033[38;5;137m";
        case 5138: return "\033[38;5;138m";
        case 5139: return "\033[38;5;139m";
        case 5140: return "\033[38;5;140m";
        case 5141: return "\033[38;5;141m";
        case 5142: return "\033[38;5;142m";
        case 5143: return "\033[38;5;143m";
        case 5144: return "\033[38;5;144m";
        case 5145: return "\033[38;5;145m";
        case 5146: return "\033[38;5;146m";
        case 5147: return "\033[38;5;147m";
        case 5148: return "\033[38;5;148m";
        case 5149: return "\033[38;5;149m";
        case 5150: return "\033[38;5;150m";
        case 5151: return "\033[38;5;151m";
        case 5152: return "\033[38;5;152m";
        case 5153: return "\033[38;5;153m";
        case 5154: return "\033[38;5;154m";
        case 5155: return "\033[38;5;155m";
        case 5156: return "\033[38;5;156m";
        case 5157: return "\033[38;5;157m";
        case 5158: return "\033[38;5;158m";
        case 5159: return "\033[38;5;159m";
        case 5160: return "\033[38;5;160m";
        case 5161: return "\033[38;5;161m";
        case 5162: return "\033[38;5;162m";
        case 5163: return "\033[38;5;163m";
        case 5164: return "\033[38;5;164m";
        case 5165: return "\033[38;5;165m";
        case 5166: return "\033[38;5;166m";
        case 5167: return "\033[38;5;167m";
        case 5168: return "\033[38;5;168m";
        case 5169: return "\033[38;5;169m";
        case 5170: return "\033[38;5;170m";
        case 5171: return "\033[38;5;171m";
        case 5172: return "\033[38;5;172m";
        case 5173: return "\033[38;5;173m";
        case 5174: return "\033[38;5;174m";
        case 5175: return "\033[38;5;175m";
        case 5176: return "\033[38;5;176m";
        case 5177: return "\033[38;5;177m";
        case 5178: return "\033[38;5;178m";
        case 5179: return "\033[38;5;179m";
        case 5180: return "\033[38;5;180m";
        case 5181: return "\033[38;5;181m";
        case 5182: return "\033[38;5;182m";
        case 5183: return "\033[38;5;183m";
        case 5184: return "\033[38;5;184m";
        case 5185: return "\033[38;5;185m";
        case 5186: return "\033[38;5;186m";
        case 5187: return "\033[38;5;187m";
        case 5188: return "\033[38;5;188m";
        case 5189: return "\033[38;5;189m";
        case 5190: return "\033[38;5;190m";
        case 5191: return "\033[38;5;191m";
        case 5192: return "\033[38;5;192m";
        case 5193: return "\033[38;5;193m";
        case 5194: return "\033[38;5;194m";
        case 5195: return "\033[38;5;195m";
        case 5196: return "\033[38;5;196m";
        case 5197: return "\033[38;5;197m";
        case 5198: return "\033[38;5;198m";
        case 5199: return "\033[38;5;199m";
        case 5200: return "\033[38;5;200m";
        case 5201: return "\033[38;5;201m";
        case 5202: return "\033[38;5;202m";
        case 5203: return "\033[38;5;203m";
        case 5204: return "\033[38;5;204m";
        case 5205: return "\033[38;5;205m";
        case 5206: return "\033[38;5;206m";
        case 5207: return "\033[38;5;207m";
        case 5208: return "\033[38;5;208m";
        case 5209: return "\033[38;5;209m";
        case 5210: return "\033[38;5;210m";
        case 5211: return "\033[38;5;211m";
        case 5212: return "\033[38;5;212m";
        case 5213: return "\033[38;5;213m";
        case 5214: return "\033[38;5;214m";
        case 5215: return "\033[38;5;215m";
        case 5216: return "\033[38;5;216m";
        case 5217: return "\033[38;5;217m";
        case 5218: return "\033[38;5;218m";
        case 5219: return "\033[38;5;219m";
        case 5220: return "\033[38;5;220m";
        case 5221: return "\033[38;5;221m";
        case 5222: return "\033[38;5;222m";
        case 5223: return "\033[38;5;223m";
        case 5224: return "\033[38;5;224m";
        case 5225: return "\033[38;5;225m";
        case 5226: return "\033[38;5;226m";
        case 5227: return "\033[38;5;227m";
        case 5228: return "\033[38;5;228m";
        case 5229: return "\033[38;5;229m";
        case 5230: return "\033[38;5;230m";
        case 5231: return "\033[38;5;231m";
        case 5232: return "\033[38;5;232m";
        case 5233: return "\033[38;5;233m";
        case 5234: return "\033[38;5;234m";
        case 5235: return "\033[38;5;235m";
        case 5236: return "\033[38;5;236m";
        case 5237: return "\033[38;5;237m";
        case 5238: return "\033[38;5;238m";
        case 5239: return "\033[38;5;239m";
        case 5240: return "\033[38;5;240m";
        case 5241: return "\033[38;5;241m";
        case 5242: return "\033[38;5;242m";
        case 5243: return "\033[38;5;243m";
        case 5244: return "\033[38;5;244m";
        case 5245: return "\033[38;5;245m";
        case 5246: return "\033[38;5;246m";
        case 5247: return "\033[38;5;247m";
        case 5248: return "\033[38;5;248m";
        case 5249: return "\033[38;5;249m";
        case 5250: return "\033[38;5;250m";
        case 5251: return "\033[38;5;251m";
        case 5252: return "\033[38;5;252m";
        case 5253: return "\033[38;5;253m";
        case 5254: return "\033[38;5;254m";
        case 5255: return "\033[38;5;255m";
        default: return "\033[0m";
    }
}

static int var_buffer[32];
static int var_count = 0;
static int var_index = 0;

void msg_term_color_buf(int first_var, ...) {
    va_list ap;
    va_start(ap, first_var);
    var_count = 0;
    int arg = first_var;
    while (arg != BUF && var_count < 32) {
        var_buffer[var_count] = arg;
        var_count++;
        arg = va_arg(ap, int);
    }
    var_index = 0;
    va_end(ap);
}

static void format_with_buffer(const char* src, char* dest, size_t dest_size) {
    size_t src_len = strlen(src);
    size_t dest_idx = 0;
    for (size_t i = 0; i < src_len && dest_idx < dest_size - 1; i++) {
        if (src[i] == '%' && i + 1 < src_len && src[i + 1] == 'd') {
            if (var_index < var_count) {
                int num = var_buffer[var_index];
                var_index++;
                char num_buf[32];
                snprintf(num_buf, sizeof(num_buf), "%d", num);
                size_t num_len = strlen(num_buf);
                if (dest_idx + num_len < dest_size - 1) {
                    strcpy(&dest[dest_idx], num_buf);
                    dest_idx += num_len;
                }
            } else {
                if (dest_idx + 2 < dest_size - 1) {
                    dest[dest_idx++] = '%';
                    dest[dest_idx++] = 'd';
                }
            }
            i++;
        } else {
            dest[dest_idx++] = src[i];
        }
    }
    dest[dest_idx] = '\0';
}

void msg_term_color(int color1, const char* text1, ...) {
    va_list ap;
    va_start(ap, text1);
    var_index = 0;
    const char* color_code = color_to_ansi(color1);
    fprintf(stderr, "%s", color_code);
    if (var_count > 0) {
        char formatted[256];
        format_with_buffer(text1, formatted, sizeof(formatted));
        fprintf(stderr, "%s", formatted);
    } else {
        fprintf(stderr, "%s", text1);
    }
    int expecting_color = 0;
    while (1) {
        if (!expecting_color) {
            int next_color = va_arg(ap, int);
            if (next_color == NUL) {
                break;
            }
            color_code = color_to_ansi(next_color);
            fprintf(stderr, "%s", color_code);
            expecting_color = 1;
        } else {
            const char* next_text = va_arg(ap, const char*);
            if (var_count > 0) {
                char formatted[256];
                format_with_buffer(next_text, formatted, sizeof(formatted));
                fprintf(stderr, "%s", formatted);
            } else {
                fprintf(stderr, "%s", next_text);
            }
            expecting_color = 0;
        }
    }
    var_count = 0;
    var_index = 0;
    fprintf(stderr, "\033[0m\n");
    va_end(ap);
}
