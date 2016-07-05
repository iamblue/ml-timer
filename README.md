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

* setInterval (Async)

``` js
  setInterval(
    function, // function
    timer,    // nubmer
  )

```

* clearTimeout

``` js

  clearTimeout(id)
```

* clearInterval

``` js

  clearInterval(id)
```

## Example

``` js
  // sync, every 2s loop this function
  __loop(function() {
    print(123);
  }, 2000);

  // ASync, every 2s loop this function
  setInterval(function() {
    print(123);
  }, 2000);

```