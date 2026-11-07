#!/usr/bin/env python3
"""Tests for the hud mod's table, config/hud.toml (openspec change
am-widescreen).

- every site's ret_va follows a direct call to one of the 2D library's
  entries (the calls src/hud_mod.c hands the layer) or to a wrapper, and
  every wrapper has a hook slot in game.toml, read against the
  analysis (analysis/disasm/: xrefs.json, functions.json and the listing);
- no duplicate sites or groups; known kinds and groups; every group used
  (these structural checks need no analysis);
- the canvas and the layout are the ones src/hud_mod.c builds (design D2,
  D3), and its draw table names the entries game.toml hooks;
- config.default.toml ships the mod on and the two display keys;
- config/hook_functions.json agrees with game.toml [mods].

  uv run pytest scripts/test_hud_table.py

The analysis is not in the repository: the tests that read it skip without
it (run `./automodellista analyze`).
"""

import json
import os
import re
import sys

import pytest

tomllib = pytest.importorskip("tomllib")

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
TABLE = os.path.join(ROOT, "config", "hud.toml")
GAME_TOML = os.path.join(ROOT, "game.toml")
DEFAULTS = os.path.join(ROOT, "packaging", "config.default.toml")
HOOK_JSON = os.path.join(ROOT, "config", "hook_functions.json")
MOD_C = os.path.join(ROOT, "src", "hud_mod.c")
ENV_H = os.path.join(ROOT, "src", "env", "recomp_env_game.h")
DISASM = os.path.join(ROOT, "analysis", "disasm")

# The 2D library's twelve entries (design D2), in src/hud_mod.c's order.
ENTRIES = [
    0x0010E180,
    0x0010E1E0,
    0x0010E210,
    0x0010E270,
    0x0010E2A0,
    0x0010E2D0,
    0x0010E300,
    0x0010E4B0,
    0x0010E540,
    0x0010E690,
    0x0010E6C0,
    0x0010E6F0,
]
KINDS = {"centre", "center", "left", "right", "proportional", "world", "stretch"}

_LINE_RE = re.compile(r"^  0x([0-9A-Fa-f]{8})  ([0-9a-fA-F]+)\s")


def load(path=TABLE):
    with open(path, "rb") as f:
        return tomllib.load(f)


def hooked():
    with open(GAME_TOML, "rb") as f:
        g = tomllib.load(f)
    return {int(v, 16) for v in g["mods"]["hook_functions"]}


def struct_errors(doc):
    """The checks that need no analysis: one line per bad row."""
    errs = []
    groups = {}
    for g in doc.get("group", []):
        if g["name"] in groups:
            errs.append("group %s: duplicate" % g["name"])
        if g.get("kind") not in KINDS:
            errs.append("group %s: unknown kind %r" % (g["name"], g.get("kind")))
        # The toolkit refuses full on groups that already stretch or keep
        # their place (docs/runtime/config.md, "HUD table").
        if "full" in g:
            if g.get("kind") in ("stretch", "world"):
                errs.append("group %s: full is for groups that move or keep" % g["name"])
            elif g["full"] not in ("stretch", "keep"):
                errs.append("group %s: full must be stretch or keep" % g["name"])
        groups[g["name"]] = g
    seen, used = set(), set()
    for s in doc.get("site", []):
        ret = s["ret_va"]
        if ret in seen:
            errs.append("site 0x%08X: duplicate" % ret)
        seen.add(ret)
        if ("group" in s) == ("kind" in s):
            errs.append("site 0x%08X: needs a group or a kind, not both" % ret)
        if "group" in s:
            used.add(s["group"])
            if s["group"] not in groups:
                errs.append("site 0x%08X: unknown group %s" % (ret, s["group"]))
        if "kind" in s and s["kind"] not in KINDS:
            errs.append("site 0x%08X: unknown kind %r" % (ret, s["kind"]))
    for name in groups:
        if name not in used:
            errs.append("group %s: no site uses it" % name)
    return errs


def entry_returns(disasm=DISASM, targets=ENTRIES):
    """The return address of every direct call to one of the targets."""
    targets = set(targets)
    with open(os.path.join(disasm, "xrefs.json"), encoding="utf-8") as f:
        froms = {
            int(x["from"], 16)
            for x in json.load(f)
            if x["type"] == "call" and int(x["to"], 16) in targets
        }
    out = set()
    asm = os.path.join(disasm, "asm")
    for name in sorted(os.listdir(asm)):
        with open(os.path.join(asm, name), encoding="utf-8", errors="replace") as f:
            for line in f:
                m = _LINE_RE.match(line)
                if m and int(m.group(1), 16) in froms:
                    out.add(int(m.group(1), 16) + len(m.group(2)) // 2)
    return out


def wrappers(doc):
    return [w["va"] for w in doc.get("wrapper", [])]


def row_errors(doc, rets):
    errs = struct_errors(doc)
    for s in doc.get("site", []):
        if s["ret_va"] not in rets:
            errs.append("site 0x%08X: does not follow a call to an entry" % s["ret_va"])
    return errs


def table_returns(doc):
    return entry_returns(targets=ENTRIES + wrappers(doc))


def test_canvas_and_layout():
    doc = load()
    assert doc["version"] == 1
    assert doc["canvas"] == {
        "w": 640,
        "h": 480,
        "origin": "top-left",
        "y": "down",
        "view": [0, 0, 640, 480],
        "target": "frame",
        "source": "fixed",
    }
    # hud_mod.c's quad: {x, y, u, v} f32, four vertices.
    assert doc["layout"] == {"stride": 16, "x_off": 0, "u_off": 8, "prim": 4}


def test_mod_hooks_the_entries():
    with open(MOD_C, encoding="utf-8") as f:
        src = f.read()
    vas = [int(v, 16) for v in re.findall(r"\{\s*(0x0010E[0-9A-Fa-f]{3})u?\s*,", src)]
    assert vas == ENTRIES
    assert set(ENTRIES) <= hooked()


def test_hook_json_matches_game_toml():
    # recomp reads config/hook_functions.json; analyze writes it from
    # game.toml [mods]. A game.toml edit without analyze leaves the slots
    # stale, so the two must agree.
    with open(GAME_TOML, "rb") as f:
        mods = tomllib.load(f)["mods"]
    with open(HOOK_JSON, encoding="utf-8") as f:
        js = json.load(f)["mods"]

    def vas(xs):
        return [int(v, 16) for v in xs]

    assert vas(js["hook_functions"]) == vas(mods["hook_functions"])
    assert vas(js["no_st0"]) == vas(mods.get("no_st0", []))
    assert vas(js.get("st0", [])) == vas(mods.get("st0", []))


def test_rows_are_well_formed():
    assert struct_errors(load()) == []


def test_bad_structure_fails():
    doc = load()
    first = doc["site"][0]["ret_va"]
    doc = dict(
        doc,
        group=list(doc["group"])
        + [
            {"name": "unused", "kind": "left"},
            {"name": "wfull", "kind": "world", "full": "stretch"},
            {"name": "badfull", "kind": "left", "full": "wide"},
        ],
        site=list(doc["site"])
        + [
            {"ret_va": first, "kind": "left"},
            {"ret_va": 0x00012346, "kind": "sideways"},
            {"ret_va": 0x00012347, "group": "nosuch"},
            {"ret_va": 0x00012348, "group": "left", "kind": "left"},
        ],
    )
    errs = "\n".join(struct_errors(doc))
    assert "0x%08X: duplicate" % first in errs
    assert "unknown kind 'sideways'" in errs
    assert "unknown group nosuch" in errs
    assert "0x00012348: needs a group or a kind" in errs
    assert "group unused: no site uses it" in errs
    assert "group wfull: full is for groups that move or keep" in errs
    assert "group badfull: full must be stretch or keep" in errs


def test_defaults_ship_the_mod_on():
    with open(DEFAULTS, "rb") as f:
        cfg = tomllib.load(f)
    assert cfg["mods"]["hud.enable"] == "on"
    assert cfg["display"]["aspect"] == "4:3"
    assert cfg["display"]["hud_aspect"] == "display"
    assert cfg["display"]["hud_margin"] == 0


def test_setting_env_has_a_row():
    # The mod API refuses a setting whose env names no row of the table.
    with open(MOD_C, encoding="utf-8") as f:
        envs = set(re.findall(r'"(RECOMP_[A-Z_]+)"', f.read()))
    with open(ENV_H, encoding="utf-8") as f:
        rows = set(re.findall(r'X\(\w+,\s*CONFIG,\s*"(RECOMP_[A-Z_]+)"', f.read()))
    assert envs == {"RECOMP_HUD"}
    assert envs <= rows


needs_analysis = pytest.mark.skipif(
    not os.path.isfile(os.path.join(DISASM, "xrefs.json")), reason="no analysis: run analyze"
)


def test_two_sided_calls_are_left_sites():
    # hud_mod.c mirrors a two-sided call's right pieces so the layer moves
    # them as left ones (design D5): the table must list it as left.
    with open(MOD_C, encoding="utf-8") as f:
        m = re.search(r"k_sides\[\]\s*=\s*\{([^}]*)\}", f.read())
    sides = [int(v, 16) for v in re.findall(r"0x([0-9A-Fa-f]+)u?", m.group(1))]
    doc = load()
    groups = {g["name"]: g["kind"] for g in doc["group"]}
    kinds = {s["ret_va"]: s.get("kind") or groups[s["group"]] for s in doc["site"]}
    assert sides
    for ret in sides:
        assert kinds.get(ret) == "left", "0x%08X" % ret


def test_wrappers_have_hook_slots():
    # The layer refuses a table whose wrapper has no hook slot.
    assert set(wrappers(load())) <= hooked()


@needs_analysis
def test_rows_follow_entry_calls():
    doc = load()
    assert row_errors(doc, table_returns(doc)) == []


@needs_analysis
def test_bad_rows_fail():
    rets = entry_returns()
    assert rets, "no call to the 2D library found"
    doc = load()
    one_off = min(rets) + 1
    doc = dict(doc, site=list(doc["site"]) + [{"ret_va": one_off, "kind": "left"}])
    errs = "\n".join(row_errors(doc, rets))
    assert "0x%08X: does not follow" % one_off in errs


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-q"]))
