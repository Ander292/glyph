compile() {
    
    local CodeDir="$1"
    local Name="$2"

    local SrcDir="$CodeDir/src"
    local IncludeDir="$CodeDir/include"
    local ObjLocal="$ObjDir/$Name"

    local Sources=()
    local Objs=()

    for file in "$SrcDir"/*.c;do
        Sources+=(`basename "$file"`)
        #ObjC+=(`basename "$file" .c`.obj)
    done

    mkdir $SrcDir 2> /dev/null
    mkdir $IncludeDir 2> /dev/null
    mkdir $ObjLocal 2> /dev/null

    local -n CompFlags="$3"
    local -n LinkFlags="$4"
    local IncFlags=(
        "-I$IncludeDir"
        "-I$SharedDir"
    )

    #echo $CodeDir
    #echo $Name
    #echo $SrcDir
    #echo $IncludeDir
    #echo $ObjLocal
    #echo ${Sources[@]}
    #echo ${IncFlags[@]}

    #echo ${Sources[@]}
    #echo ${CompFlags[@]}
    #echo ${LinkFlags[@]}


    #compiling
    for file in ${Sources[@]};do
        local CurrentObj=`basename "$file" .c`.obj
        local args="$SrcDir/$file -c ${IncFlags[@]} ${CompFlags[@]} -o $ObjLocal/$CurrentObj"
        #echo "$args"
        $gccPath $args
    done

    #linking
    for file in "$ObjLocal"/*.obj;do
        Objs+=( $file )
    done


    echo "Linking $2"
    $gccPath \
        "${Objs[@]}" \
        "${IncFlags[@]}" \
        "${LinkFlags[@]}" \
        -o "$BinDir/$Name"
}

mode="$full"
#full, run, build

gccPath="gcc-15"

if [ $# -ne 1 ]; then
    echo "Usage: `basename $0` <mode>"
    exit 1
else
    mode=$1
fi

Program="glyph"

RootDir=`pwd`

BuildDir="$RootDir/build"
BinDir="$BuildDir/bin"
ObjDir="$BuildDir/obj"

ProgramDir=$RootDir
OutputExe="$BuildDir/$Program"

#echo $AssemblerDir
#echo $EmulatorDir
#echo $SharedDir
#echo $AssetsDir
#echo $BuildDir
#echo $BinDir
#echo $ObjDir
#echo $OutputExeA
#echo $OutputExeE

mkdir "$BuildDir" 2> /dev/null
mkdir "$BinDir" 2> /dev/null
mkdir "$ObjDir" 2> /dev/null
mkdir "$EmulatorDir" 2> /dev/null
mkdir "$AssetsDir" 2> /dev/null
mkdir "$SharedDir" 2> /dev/null

CompilationFlags=(
    #"-Wall"
    #"-pedantic"
    "-Wextra"
    #"-Wconversion"
    #"-Wundef"
    #"-Wstrict-overflow=5"
    "-fdiagnostics-show-option"
    #"-g"
    "-DLINUX"
    "-Darm"
    "-O2"
    #"-fsanitize=address"
)

LinkerFlags=(
    "-pedantic"
#    "-m64"
)

case "$mode" in
    "build")
        compile "$ProgramDir" "$Program" "CompilationFlags" "LinkerFlags"
        ;;
    "run")
        # Not done yet
        ;;
    "full")
        compile "$ProgramDir" "$Program" "CompilationFlags" "LinkerFlags"
        ;;
esac
