#!/usr/bin/bash

VENV=venv

if [ ! -d "$VENV" ]
then

    PYTHON=`which python2.7`
    echo $PYTHON
    if [ ! -f $PYTHON ]
    then
        echo "could not find python"
    fi
    virtualenv -p $PYTHON $VENV

fi

$pwd.
./$VENV/bin/activate

pip install -r requirements.txt
