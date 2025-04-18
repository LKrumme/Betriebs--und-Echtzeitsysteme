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

function fileExtension {
    if [ $(echo $1 | grep ".txt") ]; then
        echo "Datei endet auf .txt" 
        while [ 1 ]
        do 
            echo -n "Soll die Datei ausgegeben werden? [y/n]:"
            read confirmation
            if [ $confirmation = "y" ] || [ $confirmation = "Y" ]; then
                cat $1
                break
            elif [ $confirmation = "n" ] || [ $confirmation = "N" ]; then
                echo "Datei wird nicht ausgegeben."
                break
            fi
        done
    fi 
}

echo "Pfad: $1"
exists $1
fileOrDir $1
isSymboliclink $1
isOwner $1
whoOwner $1
fileExtension $1