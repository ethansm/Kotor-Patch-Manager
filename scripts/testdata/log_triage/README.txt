Fixtures for scripts/test_log_triage.py (generic log summarizer scripts/log_triage.py).
Real excerpts were cut with sed from the live logs in "$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II"
on 2026-10-04 (live files were only read).

hotbar_excerpt.txt            real: force_power_hotbar_log.txt lines 1-30 + 3500-3506 ("[tick fN] " prefix; "cfg: loaded"/"img: loaded"
                              lines that must NOT split; one genuine "invalid object id" alert)
buffhud_excerpt.txt           real: buffhud_log.txt lines 1-25 (bare "buffhud loaded" banner, COLUMN#N lines)
shieldprobe_excerpt.txt       real: shieldprobe_log.txt lines 1-30 + 2093-2097 ("[t=N f=N] " prefix, hexdump continuation lines,
                              no banner, "saferead: 0 failed probes" zero-counter that must NOT alert)
chareffects_excerpt.txt       real: chareffects_log.txt lines 1-20 + 126-150 ("HH:MM:SS.mmm " prefix, two launches, "INI loaded:" lines)
equipiconfix_excerpt.txt      real: equipiconfix_log.txt lines 1-45 ("[HH:MM:SS.mmm tick] " prefix, STATS lines, two launches)
hangprobe_excerpt.txt         real: hangprobe_log.txt lines 1-50 (three launches; banner contains the word "hang" and must not alert)
levelup_excerpt.txt           real: levelup_override_log.txt lines 1-40 ("[tick] " prefix, "== Name x loaded ==" banner, "CONFIG loaded" lines)
kplog_mouseprobe_excerpt.txt  real: mouse-probe_log.txt lines 1-24 (KpLog "# kplog session" header, "T=ms f=- patch/chan" lines)
kplog_letterbox_excerpt.txt   real: k2-dialogue-letterbox-fix_log.txt lines 1-20 (KpLog header followed by the patch's own "loaded" line:
                              must stay ONE launch)
synth_multi.txt               synthesized: 3 launches, duplicate alert signatures, zero-counter line, "limit reached" last line
empty.txt                     synthesized: zero bytes
