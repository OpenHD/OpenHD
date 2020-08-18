set_bash_prompt(){
    boot_fs_mode=$(mount | sed -n -e "s/^\/dev\/.* on \/boot .*(\(r[w|o]\).*/\1/p")
	root_fs_mode=$(mount | sed -n -e "s/^\/dev\/.* on \/ .*(\(r[w|o]\).*/\1/p")
    PS1='\[\033[01;32m\]\u@\h${boot_fs_mode:+(boot:$boot_fs_mode,root:$root_fs_mode)}\[\033[00m\]:\[\033[01;34m\]\w\[\033[00m\]\$ '
}

alias ro='mount -o remount,ro /boot'
alias rw='mount -o remount,rw /boot'

# setup fancy prompt"
PROMPT_COMMAND=set_bash_prompt
