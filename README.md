# semop
opinionated utility for easy use of SysV semaphore sets

## Opinions
* SysV semaphores represent a pinnacle of IPC
* For most uses, a set of a single semaphore is adequate
* Shell scripts can benefit from semaphores
  * flock only goes so far
* Four letter identifiers are better than numeric ids

## To build from source
CFLAGS=-O3 make semop

## Usage
    Usage: ./semop [OPTION] <semaphore ascii key>
      -i increment
      -d decrement, will wait
      -D decrement, will not wait (exits 1 on EAGAIN)
      -1 set to 1
      -z set to 0
      -0 return when 0, will wait
      -x delete semaphore set
      -p print current value
      -n print current value only if set exists
    
    Usage: ./semop -s <int> <semaphore ascii key>
      -s  set to <int>
    
    If no option is given, -p is assumed.
    key is 1 to 4 ascii characters to be converted to a numeric id.
