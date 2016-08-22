# ml-timer

## API

* __loop (Sync)

``` js
  __loop(
    function, // function
    timer,    // nubmer
  )

```

* setTimeout (Async)

``` js
  setTimeout(
    function, // function
    timer,    // nubmer
  )

```


## Example

``` js
  // Async, every 2s loop this function
  setInterval(function() {
    print(123);
  }, 2000);

  // Sync, every 2s loop this function
  __loop(function() {
    print(123);
  }, 2000);

```