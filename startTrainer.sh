#windows version:
echo "starting instance of pokemonAI in directory \"${1}\" ..."
sleep ${1}
./x64/Release/pokemonAI "teamDirectory trainer${1}" "networkDirectory network${1}" 3>&1 1>&2 2>&3 | tee -a stderr_${1}.log

#linux version:
#echo "starting instance of pokemonAI in directory \"${1}\" ..."
#sleep ${1}
#cd /home/ubermouser/downloads/programming/pokemonAI/
#./dist/Release/GNU-Linux-x86/pokemonai "teamDirectory trainer${1}" "networkDirectory network${1}" 3>&1 1>&2 2>&3 | tee -a stderr_${1}.log
