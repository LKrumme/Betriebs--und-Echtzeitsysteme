#!/bin/sh

function exists {
    if [ -e $1 ]; then
        echo "Die Datei oder der Ordner existiert"
    else
        echo "Die Datei oder der Ordner existiert nicht."
    fi
}

function fileOrDir {
    if [ -f $1 ]; then 
        echo "Es handelt sich um eine Datei"
    elif [ -d $1 ]; then
        echo "Es handelt sich um einen Ordner"
    else
        echo "Irgendwas ist falsch"
    fi
}

function isSymboliclink {
    if [ -L $1 ]; then
        echo "Es handelt sich um einen symbolischen Link"
    else 
        echo "Es handelt sich nicht um einen symbolischen Link"
    fi
}

function isOwner {
    if [ -O $1 ]; then
        echo "Der Aufrufer ist Besitzer der Datei"
    else 
        echo "Der Aufrufer ist nicht Besitzer der Datei"
    fi
}

function whoOwner { 
    echo -n "Der Besitzer der Datei oder des Ordners ist: "
    ls -o -d $1 | awk '{ print $3 }'
}

for path in $@
do
echo "Pfad: $path"
exists $path
fileOrDir $path
isSymboliclink $path
isOwner $path
whoOwner $path
done 