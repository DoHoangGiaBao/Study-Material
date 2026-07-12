savedcmd_simple.mod := printf '%s\n'   simple.o | awk '!x[$$0]++ { print("./"$$0) }' > simple.mod
