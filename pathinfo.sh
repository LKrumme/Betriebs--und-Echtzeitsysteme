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


exists $1
fileOrDir $1
isSymboliclink $1