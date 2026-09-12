$MAIN_BRANCH = "main"
$REMOTE = "origin"


# FUNCIONES AUXILIARES 
function Write-ErrorAndExit {
    param ([string]$Message)

    Write-Host ""
    Write-Host "ERROR: $Message" -ForegroundColor Red
    Write-Host ""
    Read-Host "Presioná Enter para salir"
    exit 1
}


function Invoke-Git {
    param (
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments,
        [string]$ErrorMessage = "El comando de Git falló."
    )

    & git @Arguments

    if ($LASTEXITCODE -ne 0) {
        Write-ErrorAndExit $ErrorMessage
    }
}


function Get-CurrentBranch {

    $branch = git branch --show-current

    if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($branch)) {
        Write-ErrorAndExit "No se pudo determinar la rama actual (¿estás en un 'detached HEAD'?)."
    }

    return $branch.Trim()
}


function Test-UncommittedChanges {

    $status = git status --porcelain

    if ($LASTEXITCODE -ne 0) {
        Write-ErrorAndExit "No se pudo comprobar el estado del repositorio."
    }

    return -not [string]::IsNullOrWhiteSpace($status)
}


function Test-Prerequisites {
    param (
        [string]$MyBranch,
        [switch]$AllowMain
    )

    git rev-parse --is-inside-work-tree *> $null
    if ($LASTEXITCODE -ne 0) {
        Write-ErrorAndExit "No estás dentro de un repositorio Git."
    }

    git remote get-url $REMOTE *> $null
    if ($LASTEXITCODE -ne 0) {
        Write-ErrorAndExit "No existe el remoto '$REMOTE'. Verificá con 'git remote -v'."
    }

    if (-not $AllowMain -and $MyBranch -eq $MAIN_BRANCH) {
        Write-ErrorAndExit "Estás en '$MAIN_BRANCH'. Cambiá a tu rama personal antes de usar esta opción (o usá la opción de repositorio solitario)."
    }

    git rev-parse --verify MERGE_HEAD 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-ErrorAndExit "Hay un merge en progreso. Resolvelo antes de continuar."
    }

    if (-not $AllowMain) {
        git show-ref --verify --quiet "refs/heads/$MAIN_BRANCH"
        if ($LASTEXITCODE -ne 0) {
            Write-ErrorAndExit "La rama '$MAIN_BRANCH' no existe localmente."
        }
    }
}


function Test-BasicRepo {

    git rev-parse --is-inside-work-tree *> $null
    if ($LASTEXITCODE -ne 0) {
        Write-ErrorAndExit "No estás dentro de un repositorio Git."
    }

    git rev-parse --verify MERGE_HEAD 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-ErrorAndExit "Hay un merge en progreso. Resolvelo antes de continuar."
    }
}


function Get-OrSetRemoteUrl {

    $currentUrl = git remote get-url $REMOTE 2>$null
    $hasRemote = ($LASTEXITCODE -eq 0)

    if ($hasRemote) {
        Write-Host ""
        Write-Host "Repositorio remoto ya configurado: " -NoNewline
        Write-Host "$currentUrl" -ForegroundColor Yellow
        $confirm = Read-Host "¿Es este el repositorio en el que estás trabajando? (s/n)"

        if ($confirm.ToLower() -eq "s") {
            return $currentUrl
        }

        Write-Host ""
        $newUrl = Read-Host "Pegá la URL del repositorio nuevo"
        if ([string]::IsNullOrWhiteSpace($newUrl)) {
            Write-Host "URL vacía. Cancelado." -ForegroundColor Yellow
            return $null
        }

        Invoke-Git -Arguments @("remote", "set-url", $REMOTE, $newUrl) `
            -ErrorMessage "No se pudo actualizar la URL del remoto."
        return $newUrl
    }
    else {
        Write-Host ""
        Write-Host "Este repositorio todavía no tiene un remoto '$REMOTE' configurado." -ForegroundColor Yellow
        $newUrl = Read-Host "Pegá la URL del repositorio"
        if ([string]::IsNullOrWhiteSpace($newUrl)) {
            Write-Host "URL vacía. Cancelado." -ForegroundColor Yellow
            return $null
        }

        Invoke-Git -Arguments @("remote", "add", $REMOTE, $newUrl) `
            -ErrorMessage "No se pudo agregar el remoto."
        return $newUrl
    }
}



# BANNER Y MENÚ

function Write-Banner {
    Clear-Host
    Write-Host ""

    Write-Host "   _____ _____ _______    __  __    _    ___ _   _ " -ForegroundColor Blue
    Write-Host "  / ____|_   _|__   __|  |  \/  |  / \  |_ _| \ | |" -ForegroundColor Blue
    Write-Host " | |  __  | |    | |     | |\/| | / _ \  | ||  \| |" -ForegroundColor White
    Write-Host " | | |_ | | |    | |     | |  | |/ ___ \ | || |\  |" -ForegroundColor White
    Write-Host " | |__| |_| |_   | |     | |  | /_/   \_\___|_| \_|" -ForegroundColor Blue
    Write-Host "  \_____|_____|  |_|     |_|  |_|                 " -ForegroundColor Blue

    Write-Host ""
    Write-Host " ============================================================" -ForegroundColor Yellow
    Write-Host "                    GIT MAIN MANAGER" -ForegroundColor Yellow
    Write-Host " ============================================================" -ForegroundColor Yellow
    Write-Host ""

    $branch = git branch --show-current

    Write-Host "  [*] Rama actual: " -NoNewline -ForegroundColor Cyan
    Write-Host "$branch" -ForegroundColor Yellow

    Write-Host ""
    Write-Host " ============================================================" -ForegroundColor Yellow
    Write-Host ""
}

function Show-Menu {

    Write-Host " ============================================" -ForegroundColor DarkGray
    Write-Host "  [1] Subir tu rama a main (commit + merge + push)" -ForegroundColor White
    Write-Host "  [2] Actualizar tu rama con los cambios de main" -ForegroundColor White
    Write-Host "  [3] Elegir archivos puntuales para subir" -ForegroundColor White
    Write-Host "  [4] Repositorio solitario (trabajas directo en main)" -ForegroundColor White
    Write-Host "  [0] Salir" -ForegroundColor White
    Write-Host " ============================================" -ForegroundColor DarkGray
    Write-Host ""

    return Read-Host "Elegi una opcion"
}



# FLUJO COMPARTIDO: MAIN <- MERGE <- PUSH -> VOLVER


function Invoke-MergeToMainAndPush {
    param ([string]$MyBranch)

    Write-Host ""
    Write-Host "Cambiando a '$MAIN_BRANCH'..." -ForegroundColor Cyan
    Invoke-Git -Arguments @("switch", $MAIN_BRANCH) -ErrorMessage "No se pudo cambiar a '$MAIN_BRANCH'."

    Write-Host ""
    Write-Host "Actualizando '$MAIN_BRANCH' desde $REMOTE..." -ForegroundColor Cyan
    Invoke-Git -Arguments @("pull", "--no-edit", $REMOTE, $MAIN_BRANCH) `
        -ErrorMessage "No se pudo actualizar '$MAIN_BRANCH'. No se realizará el merge."

    Write-Host ""
    Write-Host "Fusionando '$MyBranch' -> '$MAIN_BRANCH'..." -ForegroundColor Cyan
    & git merge --no-edit $MyBranch

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "ERROR: El merge falló, probablemente por conflictos." -ForegroundColor Red
        Write-Host "Quedaste en '$MAIN_BRANCH' con el merge sin terminar." -ForegroundColor Yellow
        Write-Host "Resolvé los conflictos y commiteá, o cancelá con: git merge --abort" -ForegroundColor Yellow
        Write-Host "Después volvé con: git switch $MyBranch" -ForegroundColor Yellow
        Write-Host ""
        Read-Host "Presioná Enter para salir"
        exit 1
    }

    Write-Host ""
    Write-Host "Subiendo '$MAIN_BRANCH' a $REMOTE..." -ForegroundColor Cyan
    & git push $REMOTE $MAIN_BRANCH

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "ERROR: El push falló. El merge local SÍ se hizo, pero no se subió." -ForegroundColor Red
        Write-Host "Reintentá con: git push $REMOTE $MAIN_BRANCH" -ForegroundColor Yellow
        Write-Host ""
        Read-Host "Presioná Enter para salir"
        exit 1
    }

    Write-Host ""
    Write-Host "Volviendo a '$MyBranch'..." -ForegroundColor Cyan
    Invoke-Git -Arguments @("switch", $MyBranch) `
        -ErrorMessage "El push se realizó, pero no se pudo volver a '$MyBranch'."

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "          OPERACION COMPLETADA" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "$MyBranch -> $MAIN_BRANCH -> $REMOTE/$MAIN_BRANCH" -ForegroundColor Green
    Write-Host ""
}



# OPCIÓN 1: SUBIR TODO (rama personal - main)


function Invoke-PushFlow {

    $myBranch = Get-CurrentBranch
    Test-Prerequisites -MyBranch $myBranch

    if (Test-UncommittedChanges) {

        Write-Host ""
        Write-Host "Tenés cambios sin guardar:" -ForegroundColor Yellow
        Write-Host ""
        git status
        Write-Host ""

        $answer = Read-Host "¿Querés crear un commit con TODOS estos cambios? (s/n)"
        if ($answer.ToLower() -ne "s") {
            Write-Host "Operación cancelada." -ForegroundColor Yellow
            return
        }

        $commitMessage = Read-Host "Mensaje del commit"
        if ([string]::IsNullOrWhiteSpace($commitMessage)) {
            Write-Host "El mensaje no puede estar vacío. Cancelado." -ForegroundColor Yellow
            return
        }

        Invoke-Git -Arguments @("add", ".") -ErrorMessage "No se pudieron agregar los archivos."
        Invoke-Git -Arguments @("commit", "-m", $commitMessage) -ErrorMessage "No se pudo crear el commit."
    }
    else {
        Write-Host ""
        Write-Host "No hay cambios pendientes. Se va a mergear tu rama tal como está." -ForegroundColor Yellow
    }

    Invoke-MergeToMainAndPush -MyBranch $myBranch
}



# OPCIÓN 2: ACTUALIZAR TU RAMA CON MAIN


function Invoke-UpdateFlow {

    $myBranch = Get-CurrentBranch
    Test-Prerequisites -MyBranch $myBranch

    if (Test-UncommittedChanges) {
        Write-ErrorAndExit "Tenés cambios sin commitear. Commiteá o guardalos (git stash) antes de actualizar tu rama."
    }

    Write-Host ""
    Write-Host "Cambiando a '$MAIN_BRANCH'..." -ForegroundColor Cyan
    Invoke-Git -Arguments @("switch", $MAIN_BRANCH) -ErrorMessage "No se pudo cambiar a '$MAIN_BRANCH'."

    Write-Host ""
    Write-Host "Actualizando '$MAIN_BRANCH' desde $REMOTE..." -ForegroundColor Cyan
    Invoke-Git -Arguments @("pull", "--no-edit", $REMOTE, $MAIN_BRANCH) `
        -ErrorMessage "No se pudo actualizar '$MAIN_BRANCH'."

    Write-Host ""
    Write-Host "Volviendo a '$myBranch'..." -ForegroundColor Cyan
    Invoke-Git -Arguments @("switch", $myBranch) -ErrorMessage "No se pudo volver a '$myBranch'."

    Write-Host ""
    Write-Host "Fusionando '$MAIN_BRANCH' -> '$myBranch'..." -ForegroundColor Cyan
    & git merge --no-edit $MAIN_BRANCH

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "ERROR: El merge falló, probablemente por conflictos." -ForegroundColor Red
        Write-Host "Quedaste en '$myBranch' con el merge sin terminar." -ForegroundColor Yellow
        Write-Host "Resolvé los conflictos y commiteá, o cancelá con: git merge --abort" -ForegroundColor Yellow
        Write-Host ""
        Read-Host "Presioná Enter para salir"
        exit 1
    }

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "     TU RAMA YA ESTÁ ACTUALIZADA" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
}



# OPCIÓN 3: SELECCIÓN DE ARCHIVOS PUNTUALES

function Invoke-SelectiveFlow {

    $myBranch = Get-CurrentBranch
    Test-Prerequisites -MyBranch $myBranch

    $statusLines = git status --porcelain

    if ([string]::IsNullOrWhiteSpace($statusLines)) {
        Write-Host ""
        Write-Host "No hay cambios pendientes para seleccionar." -ForegroundColor Yellow
        return
    }

    $files = @($statusLines -split "`n" | Where-Object { $_.Trim() -ne "" } | ForEach-Object {
        $_.Substring(3).Trim()
    })

    $selected = @{}
    for ($i = 0; $i -lt $files.Count; $i++) { $selected[$i] = $false }

    while ($true) {
        Write-Host ""
        Write-Host " Archivos con cambios:" -ForegroundColor Cyan
        Write-Host ""
        for ($i = 0; $i -lt $files.Count; $i++) {
            $mark = if ($selected[$i]) { "[x]" } else { "[ ]" }
            Write-Host ("  {0} {1}) {2}" -f $mark, ($i + 1), $files[$i])
        }
        Write-Host ""
        Write-Host " Escribí números separados por espacio para marcar/desmarcar (ej: 1 3 4)." -ForegroundColor DarkGray
        Write-Host " Escribí 'ok' cuando termines, o 'cancelar' para salir." -ForegroundColor DarkGray
        Write-Host ""

        $userInput = Read-Host "Selección"

        if ($userInput.ToLower() -eq "cancelar") {
            Write-Host "Operación cancelada." -ForegroundColor Yellow
            return
        }

        if ($userInput.ToLower() -eq "ok") {
            break
        }

        $numbers = $userInput -split "\s+" | Where-Object { $_ -match '^\d+$' }
        foreach ($num in $numbers) {
            $idx = [int]$num - 1
            if ($idx -ge 0 -and $idx -lt $files.Count) {
                $selected[$idx] = -not $selected[$idx]
            }
        }
    }

    $chosenFiles = @()
    for ($i = 0; $i -lt $files.Count; $i++) {
        if ($selected[$i]) { $chosenFiles += $files[$i] }
    }

    if ($chosenFiles.Count -eq 0) {
        Write-Host "No seleccionaste ningún archivo. Cancelado." -ForegroundColor Yellow
        return
    }

    Write-Host ""
    Write-Host "Vas a subir estos archivos:" -ForegroundColor Cyan
    $chosenFiles | ForEach-Object { Write-Host "  - $_" }
    Write-Host ""

    $commitMessage = Read-Host "Mensaje del commit"
    if ([string]::IsNullOrWhiteSpace($commitMessage)) {
        Write-Host "El mensaje no puede estar vacío. Cancelado." -ForegroundColor Yellow
        return
    }

    Invoke-Git -Arguments (@("add") + $chosenFiles) -ErrorMessage "No se pudieron agregar los archivos seleccionados."
    Invoke-Git -Arguments @("commit", "-m", $commitMessage) -ErrorMessage "No se pudo crear el commit."

    Invoke-MergeToMainAndPush -MyBranch $myBranch
}



# OPCIÓN 4: REPOSITORIO SOLITARIO (trabajás directo en main)


function Invoke-SoloFlow {

    Test-BasicRepo
    $branch = Get-CurrentBranch

    Write-Host ""
    Write-Host "Modo repositorio solitario: vas a trabajar directo sobre '$branch'." -ForegroundColor Cyan

    $remoteUrl = Get-OrSetRemoteUrl
    if ($null -eq $remoteUrl) {
        return
    }

    # Traer primero lo que haya en el remoto, por si trabajaste desde otra compu
    Write-Host ""
    Write-Host "Actualizando '$branch' desde $REMOTE..." -ForegroundColor Cyan
    & git pull --no-edit $REMOTE $branch

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "ERROR: No se pudo actualizar '$branch' desde $REMOTE." -ForegroundColor Red
        Write-Host "Si es la primera vez que subís este repo, puede que el remoto todavía no tenga esta rama." -ForegroundColor Yellow
        $skip = Read-Host "¿Querés continuar de todos modos y forzar el intento de push? (s/n)"
        if ($skip.ToLower() -ne "s") {
            Write-Host "Operación cancelada." -ForegroundColor Yellow
            return
        }
    }

    if (Test-UncommittedChanges) {

        Write-Host ""
        Write-Host "Tenés cambios sin guardar:" -ForegroundColor Yellow
        Write-Host ""
        git status
        Write-Host ""

        $answer = Read-Host "¿Querés crear un commit con TODOS estos cambios? (s/n)"
        if ($answer.ToLower() -ne "s") {
            Write-Host "Operación cancelada. No se creó ningún commit." -ForegroundColor Yellow
            return
        }

        $commitMessage = Read-Host "Mensaje del commit"
        if ([string]::IsNullOrWhiteSpace($commitMessage)) {
            Write-Host "El mensaje no puede estar vacío. Cancelado." -ForegroundColor Yellow
            return
        }

        Invoke-Git -Arguments @("add", ".") -ErrorMessage "No se pudieron agregar los archivos."
        Invoke-Git -Arguments @("commit", "-m", $commitMessage) -ErrorMessage "No se pudo crear el commit."
    }
    else {
        Write-Host ""
        Write-Host "No hay cambios pendientes para commitear." -ForegroundColor Yellow
    }

    Write-Host ""
    Write-Host "Subiendo '$branch' a $REMOTE..." -ForegroundColor Cyan
    & git push $REMOTE $branch

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "ERROR: El push falló." -ForegroundColor Red
        Write-Host "Reintentá manualmente con: git push $REMOTE $branch" -ForegroundColor Yellow
        Write-Host ""
        Read-Host "Presioná Enter para salir"
        exit 1
    }

    Write-Host ""
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "          OPERACION COMPLETADA" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "$branch -> $REMOTE/$branch" -ForegroundColor Green
    Write-Host ""
}



# LOOP PRINCIPAL


while ($true) {

    Write-Banner
    $option = Show-Menu

    switch ($option) {
        "1" { Invoke-PushFlow }
        "2" { Invoke-UpdateFlow }
        "3" { Invoke-SelectiveFlow }
        "4" { Invoke-SoloFlow }
        "0" { exit 0 }
        default { Write-Host "Opción inválida." -ForegroundColor Red }
    }

    Write-Host ""
    Read-Host "Presioná Enter para volver al menú"
}
