# Git Manager

**Git Manager** is an **Open Source** project designed to simplify and automate common **Git and GitHub** tasks.

Its goal is to make repository management easier by reducing the number of commands users need to execute manually.

## Installation and Usage

If Windows prevents Git Manager from running, follow these steps.

### 1. Open PowerShell

Make sure you run the commands from a **PowerShell** window.

You can use:

* The integrated terminal in **Visual Studio Code**.
* A native **PowerShell** window in Windows.

### 2. Initial Setup

These steps only need to be performed **once per computer**.

First, enable the execution of local scripts by running:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

If PowerShell asks for confirmation, type `Y` and press **Enter**.

Then, unblock the script:

```powershell
Unblock-File .\git-main.ps1
```

This is necessary because **Windows may block scripts downloaded from the Internet for security reasons**. This command allows PowerShell to run the script.

### 3. Run Git Manager

Once the initial setup is complete, you do not need to repeat the previous steps.

Every time you want to use Git Manager, run:

```powershell
.\git-main.ps1
```

The script will then automate the Git tasks configured in the project.

> **Note:** If the script still does not run after following these steps, make sure you are currently in the directory containing `git-main.ps1`.

**Date:** September 7, 2026
