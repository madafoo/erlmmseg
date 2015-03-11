-module(erlmmseg).
-on_load(on_load/0).

-export([
         load/0,
         load/1,
         seg/1
        ]).

-define(MMSEG_DICT_DIR, "/usr/local/etc").

on_load() ->
    File = filename:join([filename:dirname(code:which(?MODULE)), "..", "priv", erlmmseg]),
    ok = erlang:load_nif(File, 0).

load() ->
    erlmmseg:load(?MMSEG_DICT_DIR).

seg(_) ->
    not_loaded.

load(_) ->
    not_loaded.
