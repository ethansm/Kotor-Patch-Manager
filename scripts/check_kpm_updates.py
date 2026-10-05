#!/usr/bin/env python3
"""
check_kpm_updates.py - Lightweight GitHub reply and notification monitor for KotOR Patch Manager.

Monitors issues, RFCs, and Pull Requests on LaneDibello/Kotor-Patch-Manager (or any specified repo)
for new replies, comments, and reviews from upstream maintainers and contributors.

Designed for low token consumption and zero noise:
- If run with --quiet and there are no new updates, outputs NOTHING (0 tokens in AI sessions).
- Uses a local JSON state file to track seen comment IDs and avoid duplicate alerts.
- Discovers all issues and PRs involving your account automatically.
- Optional desktop notifications via notify-send.
"""

import sys
import os
import json
import argparse
import subprocess
from datetime import datetime, timezone

DEFAULT_REPO = "LaneDibello/Kotor-Patch-Manager"
DEFAULT_STATE_DIR = os.path.expanduser("~/.config/kpm_tracker")
DEFAULT_STATE_FILE = os.path.join(DEFAULT_STATE_DIR, "state.json")


def run_gh_json(args):
    """Run a gh CLI command and return parsed JSON."""
    cmd = ["gh"] + args
    try:
        res = subprocess.run(cmd, capture_output=True, text=True, check=True)
        return json.loads(res.stdout)
    except subprocess.CalledProcessError as e:
        sys.stderr.write(f"[Error] gh command failed: {' '.join(cmd)}\n{e.stderr}\n")
        return None
    except json.JSONDecodeError as e:
        sys.stderr.write(f"[Error] Failed to parse JSON from gh command: {' '.join(cmd)}\n{e}\n")
        return None


def run_gh_text(args):
    """Run a gh CLI command and return stripped stdout string."""
    cmd = ["gh"] + args
    try:
        res = subprocess.run(cmd, capture_output=True, text=True, check=True)
        return res.stdout.strip()
    except subprocess.CalledProcessError as e:
        sys.stderr.write(f"[Error] gh command failed: {' '.join(cmd)}\n{e.stderr}\n")
        return None


def get_current_user():
    """Get the authenticated GitHub login name."""
    user = run_gh_text(["api", "user", "-q", ".login"])
    return user if user else "ethansm"


def load_state(path):
    """Load persistent state from JSON file."""
    if os.path.exists(path):
        try:
            with open(path, "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            sys.stderr.write(f"[Warning] Could not read state file {path}: {e}\n")
    return {
        "last_check": None,
        "seen_comment_ids": [],
        "tracked_items": {},  # str(number) -> {"title": ..., "type": "issue"|"pr", "comments": int}
        "last_discovery": None,
    }


def save_state(path, state):
    """Save persistent state to JSON file."""
    os.makedirs(os.path.dirname(path), exist_ok=True)
    temp_path = f"{path}.tmp"
    with open(temp_path, "w", encoding="utf-8") as f:
        json.dump(state, f, indent=2, sort_keys=True)
    os.replace(temp_path, path)


def discover_user_items(repo, user):
    """Discover all issues and PRs involving the user in the repository."""
    tracked = {}
    
    # 1. Issues
    issues = run_gh_json([
        "search", "issues",
        "--repo", repo,
        f"involves:{user}",
        "--json", "number,title,url,commentsCount,updatedAt,state"
    ])
    if issues:
        for i in issues:
            tracked[str(i["number"])] = {
                "title": i["title"],
                "url": i["url"],
                "type": "issue",
                "comments": i.get("commentsCount", 0),
                "updated_at": i.get("updatedAt", ""),
                "state": i.get("state", "OPEN")
            }
            
    # 2. PRs
    prs = run_gh_json([
        "search", "prs",
        "--repo", repo,
        f"involves:{user}",
        "--json", "number,title,url,commentsCount,updatedAt,state"
    ])
    if prs:
        for p in prs:
            tracked[str(p["number"])] = {
                "title": p["title"],
                "url": p["url"],
                "type": "pr",
                "comments": p.get("commentsCount", 0),
                "updated_at": p.get("updatedAt", ""),
                "state": p.get("state", "OPEN")
            }
            
    return tracked


def send_desktop_notification(title, body):
    """Send a native Linux desktop notification via notify-send if available."""
    try:
        subprocess.run(
            ["notify-send", "-a", "KPM GitHub Tracker", "-u", "normal", title, body],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False
        )
    except Exception:
        pass


def main():
    parser = argparse.ArgumentParser(description="Monitor GitHub issues and PRs for replies on KotOR Patch Manager.")
    parser.add_argument("--repo", default=DEFAULT_REPO, help=f"GitHub repository to check (default: {DEFAULT_REPO})")
    parser.add_argument("--user", default=None, help="GitHub username to monitor (defaults to authenticated user)")
    parser.add_argument("--quiet", "-q", action="store_true", help="Only output if there are new updates (0 output if quiet)")
    parser.add_argument("--summary", "-s", action="store_true", help="Always print a 1-line summary when up-to-date")
    parser.add_argument("--refresh", "-r", action="store_true", help="Force re-discovery of issues/PRs involving the user")
    parser.add_argument("--notify", action="store_true", default=True, help="Send desktop notification on new comments (default: True)")
    parser.add_argument("--no-notify", dest="notify", action="store_false", help="Disable desktop notifications")
    parser.add_argument("--list", "-l", action="store_true", help="List all currently tracked issues and PRs")
    parser.add_argument("--state-file", default=DEFAULT_STATE_FILE, help=f"State file path (default: {DEFAULT_STATE_FILE})")
    parser.add_argument("--mark-seen", action="store_true", help="Mark all existing comments as seen without alerting")
    parser.add_argument("--since", default=None, help="Check comments since specific ISO timestamp (e.g. 2026-10-02T20:00:00Z)")
    args = parser.parse_args()

    user = args.user or get_current_user()
    state = load_state(args.state_file)
    seen_ids = set(state.get("seen_comment_ids", []))
    now_iso = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

    # Discover or refresh tracked items if empty or requested
    if args.refresh or not state.get("tracked_items"):
        state["tracked_items"] = discover_user_items(args.repo, user)
        state["last_discovery"] = now_iso

    tracked = state.get("tracked_items", {})

    if args.list:
        print(f"=== Tracked Items on {args.repo} for @{user} ({len(tracked)} items) ===")
        sorted_nums = sorted([int(n) for n in tracked.keys()], reverse=True)
        for num in sorted_nums:
            item = tracked[str(num)]
            kind = "PR" if item.get("type") == "pr" else "Issue"
            print(f"  #{num:>3} [{kind:<5}] ({item.get('comments', 0):>2} comments) {item.get('title')}")
        return 0

    # Determine 'since' timestamp
    since = args.since or state.get("last_check")
    # If this is the very first run and no since is provided, default to past 24 hours
    if not since:
        since = "2026-10-01T00:00:00Z"

    # Query issues comments
    issue_comments = run_gh_json(["api", f"repos/{args.repo}/issues/comments?since={since}"]) or []
    # Query PR review comments
    pr_comments = run_gh_json(["api", f"repos/{args.repo}/pulls/comments?since={since}"]) or []

    all_comments = issue_comments + pr_comments
    new_replies = []

    for c in all_comments:
        c_id = c.get("id")
        c_user = c.get("user", {}).get("login", "")
        # Ignore our own comments
        if c_user.lower() == user.lower():
            seen_ids.add(c_id)
            continue

        # Extract issue/PR number
        # For issue comments: c['issue_url'] ends with /issues/{num}
        # For pull comments: c['pull_request_url'] ends with /pulls/{num}
        target_url = c.get("issue_url") or c.get("pull_request_url") or ""
        if not target_url:
            continue
        try:
            num = str(int(target_url.split("/")[-1]))
        except ValueError:
            continue

        # Check if it belongs to one of our tracked items
        if num in tracked:
            if c_id not in seen_ids:
                new_replies.append({
                    "id": c_id,
                    "num": num,
                    "item": tracked[num],
                    "user": c_user,
                    "created_at": c.get("created_at"),
                    "body": c.get("body", "").strip(),
                    "html_url": c.get("html_url") or f"https://github.com/{args.repo}/issues/{num}#issuecomment-{c_id}",
                })
                seen_ids.add(c_id)

    # If --mark-seen, update state without alerting
    if args.mark_seen:
        state["seen_comment_ids"] = list(seen_ids)
        state["last_check"] = now_iso
        save_state(args.state_file, state)
        print(f"Marked {len(all_comments)} comments as seen. State updated to {now_iso}.")
        return 0

    # Output results
    if new_replies:
        print(f"\n🔔 \033[1;32m[KPM GitHub Updates]\033[0m Found {len(new_replies)} new reply on {args.repo}:\n")
        for r in new_replies:
            num = r["num"]
            title = r["item"].get("title", "")
            commenter = r["user"]
            created = r["created_at"]
            body = r["body"]
            # Truncate preview to 4 lines or 280 chars
            preview_lines = body.splitlines()[:4]
            preview = "\n    ".join(preview_lines)
            if len(body.splitlines()) > 4 or len(preview) > 280:
                preview = preview[:280] + "..."

            print(f"  \033[1;36m#{num}\033[0m \033[1m{title}\033[0m")
            print(f"  Comment by \033[1;33m@{commenter}\033[0m at {created}:")
            print(f"    {preview}")
            print(f"  \033[4;34m{r['html_url']}\033[0m\n")

            if args.notify:
                send_desktop_notification(
                    f"KPM Reply: #{num} by @{commenter}",
                    f"{title}\n\n\"{preview_lines[0][:100]}\""
                )
    else:
        if args.summary:
            print(f"✓ \033[1;30m[KPM Tracker]\033[0m All quiet on {args.repo} ({len(tracked)} items tracked). Last checked: {now_iso}")
        elif not args.quiet:
            print(f"No new replies found on {args.repo} since {since}.")

    # Update state
    state["seen_comment_ids"] = list(seen_ids)
    state["last_check"] = now_iso
    save_state(args.state_file, state)

    return 0


if __name__ == "__main__":
    sys.exit(main())
