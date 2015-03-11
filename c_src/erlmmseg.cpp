#include <set>
#include <string>
#include <vector>

#include <mmseg/Segmenter.h>
#include <mmseg/SegmenterManager.h>

#include <erl_nif.h>

const char* skipWords_arr[] = {
    " ","\r","\n","\t",
    "　","“","”", "《", "》",
    ",",".","!","@","#","$","%","^","&","*","(",")","-","=","_","+",
    "\"","'","~",";",":","<",">","/","?","[","]","{","}","\\","|",
    "！","￥","⋯","（","）","—","【","】","。","，","、","？","「","」",
    NULL,
};

uint32_t decodeUTF8(const char* cp){
    unsigned char c = *cp;
    if(c<0x80) return c;
    if(c<0xE0) return ((c & 0x1F) << 6) |  (cp[1] & 0x3F);
    if(c<0xF0) return ((c & 0x0F) <<12) | ((cp[1] & 0x3F) <<  6) |  (cp[2] & 0x3F);
    if(c<0xF8) return ((c & 0x07) <<18) | ((cp[1] & 0x3F) << 12) | ((cp[2] & 0x3F) <<  6) |  (cp[3] & 0x3F);
    if(c<0xFC) return ((c & 0x03) <<24) | ((cp[1] & 0x3F) << 18) | ((cp[2] & 0x3F) << 12) | ((cp[3] & 0x3F) <<  6) |  (cp[4] & 0x3F);
    return ((c & 0x01) <<30) | ((cp[1] & 0x3F) << 24) | ((cp[2] & 0x3F) << 18) | ((cp[3] & 0x3F) << 12) | ((cp[4] & 0x3F) << 6) | (cp[5] & 0x3F);
}

std::set<uint32_t> skipWords;
css::SegmenterManager g_mgr;

static int init(const char *path) {
    if(g_mgr.init(path) != 0) {
        return 1;
    }
    const char** sw = skipWords_arr;
    while(*sw){
        skipWords.insert(decodeUTF8(*sw));
        sw++;
    }
    return 0;
}

bool isSkipWord(const char* cp) {
    return skipWords.find(decodeUTF8(cp)) != skipWords.end();
}

extern "C"{
static ERL_NIF_TERM load(ErlNifEnv *env, int, const ERL_NIF_TERM argv[]) {
    char file[512];
    enif_get_string(env, argv[0], file, sizeof(file), ERL_NIF_LATIN1);
    int ret = init(file);
    if (ret)
        return enif_make_badarg(env);
    return enif_make_atom(env, "ok");
}

static int upgrade(ErlNifEnv*, void**, void**, ERL_NIF_TERM) {
    return 0;
}

static ERL_NIF_TERM seg(ErlNifEnv *env, int, const ERL_NIF_TERM argv[])
{
    ErlNifBinary bin; 
    std::vector<ERL_NIF_TERM> terms;
    if(!enif_inspect_binary(env, argv[0], &bin)) {
        return enif_make_list_from_array(env, &terms[0], terms.size());
    }

    css::Segmenter* seg = g_mgr.getSegmenter(false);
    if(seg == NULL) {
        return enif_make_list_from_array(env, &terms[0], terms.size());
    }

    char *text = (char*) bin.data;
    size_t size = bin.size;
    seg->setBuffer((u1*)text, size);

    u2 len, symlen;
    while(1) { 
        len = symlen = 0;
        char* tok = (char*)seg->peekToken(len, symlen);
        if(!tok || !*tok || !len){
            break;
        }
        // We will ignore the single utf8 word
        if(isSkipWord(tok) || (len == 3 && (unsigned char)tok[0] > 127)){
            seg->popToken(len);
            continue;
        }
        ErlNifBinary binText;
        enif_alloc_binary(len, &binText);
        memcpy(binText.data, tok, len);
        terms.push_back(enif_make_binary(env, &binText));
        seg->popToken(len);
    }
    delete seg;
    return enif_make_list_from_array(env, &terms[0], terms.size());
}

static ErlNifFunc mmsegnif_funcs[] =
{
    {"load", 1, load, 0},
    {"seg", 1, seg, 0},
};
}

ERL_NIF_INIT(erlmmseg, mmsegnif_funcs, NULL, NULL, upgrade, NULL);
