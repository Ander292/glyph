param (
    [string]$mode = "full"
)
# full, run, build, buildP

$ProjectName = "glyph"

# Directories

$RootDir            = Split-Path -Parent    $MyInvocation.MyCommand.Path

$SrcDir             = Join-Path $RootDir    "src"
$IncludeDir         = Join-Path $RootDir    "include"
$AssetsDir          = Join-Path $RootDir    "assets"
$ResourceDir        = Join-Path $RootDir    "resources"

$BuildDir           = Join-Path $RootDir    "build"
$BinDir             = Join-Path $BuildDir   $ProjectName
$ObjDir             = Join-Path $BuildDir   "obj"

$ResourcesObj       = Join-Path $ObjDir     "resource.obj"
$OutputExe          = Join-Path $BinDir     "$ProjectName.exe"

# Source arrays

[array]$Sources     = Get-ChildItem "$SrcDir\*.c"       | ForEach-Object { $_.FullName }

[array]$ObjC        = Get-ChildItem "$SrcDir\*.c"       | ForEach-Object { $_.BaseName + ".obj"}

[array]$Resources   = Get-ChildItem "$ResourceDir\*.rc" | ForEach-Object { $_.FullName }


#Making folders if they arent present:

New-Item -ItemType Directory -Force -Path $SrcDir       | Out-Null
New-Item -ItemType Directory -Force -Path $IncludeDir   | Out-Null
New-Item -ItemType Directory -Force -Path $AssetsDir    | Out-Null
New-Item -ItemType Directory -Force -Path $ResourceDir  | Out-Null

New-Item -ItemType Directory -Force -Path $BuildDir     | Out-Null
New-Item -ItemType Directory -Force -Path $BinDir       | Out-Null
New-Item -ItemType Directory -Force -Path $ObjDir       | Out-Null

#Flags that are included based on which libraries are there
$IncludeFlags = @(
    "-I$IncludeDir"
)

#Flags for special libraries (if there are any)
$LibFlags = @(
)

$CompilationFlagsOrd = @(
    #"-Wall"
    #"-pedantic"
    "-Wextra"
    #"-Wconversion"
    #"-Wundef"
    #"-Wstrict-overflow=5"
    "-fdiagnostics-show-option"
    "-g"
    #"-O2"
    #"-fsanitize=address"
)

$CompilationFlagsProd = @(
    #"-Wall"
    #"-pedantic"
    "-Wextra"
    #"-Wconversion"
    #"-Wundef"
    #"-Wstrict-overflow=5"
    "-fdiagnostics-show-option"
    #"-g"
    "-O2"
    #"-fsanitize=address"
)

$CompilationFlags = @()

if( ($mode -eq "build") -OR ($mode -eq "full") ){
    $CompilationFlags = $CompilationFlagsOrd
}
elseif( ($mode -eq "buildP") ){
    $CompilationFlags = $CompilationFlagsProd
}

# Extra flags for the linking stage
$ExtraFlags = @(
    "-pedantic"
    "-m64"
)

if( ( $mode -eq "full" ) -OR ( $mode -eq "build" ) -OR ( $mode -eq "buildP") ){

    # Compiling c code into obj files
    Write-Host "Compiling c files..."
    $i = 0
    $Sources.ForEach({
        $CurrentObj     = Join-Path     $ObjDir  $ObjC[$i]

        gcc `
            $IncludeFlags `
            $_ `
            $CompilationFlags `
            -c `
            -o `
            $CurrentObj
        
        $i++
        if ($LASTEXITCODE -ne 0) {Write-Error "Error compiling $_"}
    })

    # Compiling resources into a windows resource file:

    Write-Host "Preparing resources..."
    windres `
        $Resources `
        -O `
        coff `
        -o `
        $ResourcesObj


    # Fetching every obj file that was produced as a result
    [array]$Objs        = Get-ChildItem "$ObjDir\*.obj"         | ForEach-Object { $_.FullName }


    # Linking stage
    Write-Host "Linking..."
    gcc `
        $Objs `
        $IncludeFlags `
        $LibFlags `
        $ExtraFlags `
        -o $OutputExe

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Compilation fajled!"
        exit(1)
    }

    Write-Host "Compilation was sucessful!"

    
}

# I added mine to path and I hate copying it to C:\bin manually
if ( $mode -eq "update" ) {
        Copy-Item $OutputExe "C:\bin\" -Force
}

if ( ( $mode -eq "full" ) -OR ( $mode -eq "run" ) ) {
    #$ProgramPath = Read-Host "Input file name to open"
    Write-Host "Running program..."
    #& "$OutputExe" $ProgramPath
    & $OutputExe "Lana.txt"
}