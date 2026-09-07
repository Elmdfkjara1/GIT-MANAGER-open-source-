# Git Manager

**Git Manager** es un proyecto **Open Source** diseñado para simplificar y automatizar tareas frecuentes de **Git y GitHub**.

Su objetivo es facilitar el manejo de repositorios, reduciendo la cantidad de comandos que el usuario debe ejecutar manualmente.

## Instalación y ejecución

Si Windows impide ejecutar Git Manager, seguí los siguientes pasos.

### 1. Abrir PowerShell

Asegurate de ejecutar los comandos desde una ventana de **PowerShell**.

Podés utilizar:

* La terminal integrada de **Visual Studio Code**.
* Una ventana de **PowerShell** de Windows.

### 2. Configuración inicial

Estos pasos deben realizarse **una sola vez por computadora**.

Primero, habilitá la ejecución de scripts locales ejecutando:

```powershell
Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
```

Si PowerShell solicita confirmación, escribí `S` y presioná **Enter**.

Después, desbloqueá el script:

```powershell
Unblock-File .\git-main.ps1
```

Esto es necesario porque **Windows puede bloquear scripts descargados de Internet por motivos de seguridad**. El comando permite que PowerShell considere el script como seguro para su ejecución.

### 3. Ejecutar Git Manager

Una vez realizada la configuración inicial, no es necesario repetir los pasos anteriores.

Cada vez que quieras utilizar Git Manager, ejecutá:

```powershell
.\git-main.ps1
```

A partir de ese momento, el script se encargará de automatizar las tareas de Git configuradas en el proyecto.

> **Nota:** Si el script sigue sin ejecutarse después de estos pasos, verificá que estés ubicado en la carpeta donde se encuentra `git-main.ps1`.
