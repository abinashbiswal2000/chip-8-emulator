# 🚀 GitHub & Git: The Ubuntu Setup Guide

---

## 1. Installation & Authentication

Before anything else, install the software and link your computer to your online account to handle security.

- **Install Git:** Run `sudo apt update && sudo apt install git`
- **Install GitHub CLI:** Run `sudo apt install gh` to manage authentication easily.
- **Login:** Run `gh auth login` and follow the interactive prompts:
  - Account: GitHub.com
  - Protocol: HTTPS
  - Authenticate Git: Yes
  - Method: Login with a web browser
- **The Handshake:** Copy the 8-character code from the terminal, press Enter to open your browser, paste the code, and click Authorize GitHub.

---

## 2. One-Time Identity Setup

Tell Git who you are. This signature gets stamped on every commit you make. Use the same email as your GitHub account so your activity is correctly linked to your profile.

```bash
git config --global user.name "Your Name"
git config --global user.email "youremail@example.com"
```

---

## 3. First-Time Project Connection

If you have a local project and want to link it to a brand-new, empty GitHub repository, run these commands inside your project folder in this exact order:

```bash
git init
git add .
git commit -m "Initial commit"
git remote add origin <URL>
git branch -M main
git push -u origin main
```

| Command | Purpose |
|---|---|
| `git init` | Turns your folder into a Git repository. Creates the hidden `.git` folder. |
| `git add .` | Stages all files, preparing them for the first snapshot. |
| `git commit -m "..."` | Creates the first snapshot locally. A commit must exist before you can push. |
| `git remote add origin <URL>` | Links your local repo to the GitHub repository URL. `origin` is just a nickname for the full URL. |
| `git branch -M main` | Renames your current branch to `main` (the modern standard). |
| `git push -u origin main` | Uploads your commits and links the branches so future pushes only need `git push`. |

---

## 4. File Visibility: What Git Sees

Use these commands to understand what Git is tracking at any point.

- **`git status`** — The most important check. Shows files in three possible states:
  - **Red (Untracked):** Git has never seen this file. It won't be saved unless you `git add` it.
  - **Red (Modified):** Git knows this file but you've changed it and haven't staged it yet.
  - **Green (Staged):** You've run `git add` on this file and it's ready to be committed.

- **`git ls-files`** — Lists every file Git is currently monitoring in your repository.

- **`git diff`** — Shows exactly what changed inside a tracked file, line by line.

---

## 5. The Daily Git Cycle

Once the project is connected, this is the only cycle you need for day-to-day work.

```bash
git add .
git commit -m "Describe what you changed"
git push
```

| Step | Command | What it does |
|---|---|---|
| Stage | `git add .` | Prepares your changes to be saved. |
| Snapshot | `git commit -m "..."` | Creates a permanent local snapshot. |
| Upload | `git push` | Uploads local snapshots to GitHub. |

> **Key idea:** `git push` is not about uploading files. It uploads **snapshots (commits)**. GitHub then renders those snapshots as files for you to browse.

---

## 6. Branching: Parallel Timelines

A branch is not a folder. It is a named pointer sitting on your commit timeline. Branching lets you experiment with new features without touching your working code.

```
Commit 1 --> Commit 2 --> Commit 3                 (jackBranch)
                               \
                                --> Commit 4        (jessicaBranch)
```

**Step A — Rename your current branch**

```bash
git branch -M jackBranch
```

Renames whatever branch you are currently on. `-M` does not specifically target `main` — it renames whichever branch you are standing on at that moment.

**Step B — Create an experimental branch**

```bash
git checkout -b jessicaBranch
```

The `-b` flag creates and steps into the new branch in one command. Any commits you make here won't affect jackBranch.

**Step C — Make commits on jessicaBranch**

```bash
git add .
git commit -m "Experimental changes on jessicaBranch"
```

**Step D — Merge jessicaBranch into jackBranch**

Always switch to the destination branch first — you are standing on jack and pulling jessica's work in.

```bash
git checkout jackBranch
git merge jessicaBranch
```

**Step E — Cleanup (Optional)**

```bash
git branch -d jessicaBranch
```

---

## 7. Quick Reference Cheatsheet

| Task | Command |
|---|---|
| Check what Git sees | `git status` |
| See all tracked files | `git ls-files` |
| See what changed in a file | `git diff` |
| Stage all changes | `git add .` |
| Create a snapshot | `git commit -m "..."` |
| Upload to GitHub | `git push` |
| Create and switch to a branch | `git checkout -b <branch-name>` |
| Switch to an existing branch | `git checkout <branch-name>` |
| Merge a branch into current | `git merge <branch-name>` |
| Delete a branch | `git branch -d <branch-name>` |
