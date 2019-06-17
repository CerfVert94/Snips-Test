#!/usr/bin/env bash -e

VENV=venv

if [ ! -d "$VENV" ]
then

    PYTHON=`which python2`
    echo $PYTHON
    if [ ! -f $PYTHON ]
    then
        echo "could not find python"
    fi
    sudo virtualenv -p $PYTHON $VENV

fi

$pwd.
sudo chmod a+x action*
sudo chmod a+x ./$VENV/bin/activate
sudo ./$VENV/bin/activate
pip install -r requirements.txt
