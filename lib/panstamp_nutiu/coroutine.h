//we're using a technique similar to Duff's device to create coroutines
//https://gist.github.com/laindir/6369535

#ifndef COROUTINE_H
#define COROUTINE_H

#define start(state) switch(state) { case 0:;
#define finish default:; }
/*save state and return value*/
#define yield(state,value) do { state = __LINE__; return (value); case __LINE__:; } while (0)

#endif