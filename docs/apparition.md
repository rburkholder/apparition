# apparition build notes

    git clone git@github.com:rburkholder/apparition.git
    cd apparition
    mkdir build
    cd build
    cmake ..
    make

# Invocation

* working directory is ./var, start the application in there
* application is currently designed to work with one mqtt broker, so command line:
  * apparition &lt;mqtt address&gt; &lt;mqtt username&gt; &lt;mqtt password&gt;"
  * topics are defined in relevant lua scripts, which are located in ./var/script



