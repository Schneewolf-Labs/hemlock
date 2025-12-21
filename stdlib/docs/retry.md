# @stdlib/retry - Retry and Backoff Utilities

Provides retry logic with configurable backoff strategies for handling transient failures in network calls, file operations, and other unreliable operations.

## Import

```hemlock
import { retry, retry_with_backoff, exponential_backoff } from "@stdlib/retry";
import { retry_linear, retry_if, retry_until } from "@stdlib/retry";
import { linear_backoff, constant_backoff, add_jitter, with_retry } from "@stdlib/retry";
```

## Backoff Strategies

### exponential_backoff(attempt, base_delay?, max_delay?, multiplier?)

Calculate exponential backoff delay.

```hemlock
exponential_backoff(0);          // 100ms (base)
exponential_backoff(1);          // 200ms
exponential_backoff(2);          // 400ms
exponential_backoff(3);          // 800ms

// Custom parameters
exponential_backoff(2, 50, 10000, 3.0);  // 450ms (50 * 3^2)
```

**Parameters:**
- `attempt`: Current attempt number (0-based)
- `base_delay`: Base delay in milliseconds (default: 100)
- `max_delay`: Maximum delay cap in milliseconds (default: 30000)
- `multiplier`: Multiplier for each attempt (default: 2.0)

**Returns:** Delay in milliseconds

### linear_backoff(attempt, base_delay?, increment?, max_delay?)

Calculate linear backoff delay.

```hemlock
linear_backoff(0);  // 100ms (base)
linear_backoff(1);  // 1100ms (100 + 1000)
linear_backoff(2);  // 2100ms (100 + 2000)

// Custom parameters
linear_backoff(2, 50, 500, 5000);  // 1050ms (50 + 2*500)
```

**Parameters:**
- `attempt`: Current attempt number (0-based)
- `base_delay`: Base delay in milliseconds (default: 100)
- `increment`: Delay increment per attempt (default: 1000)
- `max_delay`: Maximum delay cap in milliseconds (default: 30000)

**Returns:** Delay in milliseconds

### constant_backoff(delay)

Return constant delay (no backoff).

```hemlock
constant_backoff(500);  // Always 500ms
```

### add_jitter(delay, jitter_factor?)

Add random jitter to a delay to prevent thundering herd.

```hemlock
add_jitter(1000);       // Random value between 900-1100ms (10% jitter)
add_jitter(1000, 0.25); // Random value between 750-1250ms (25% jitter)
```

**Parameters:**
- `delay`: Base delay in milliseconds
- `jitter_factor`: Jitter factor (0.0 to 1.0, default: 0.1)

**Returns:** Delay with random jitter applied

## Retry Functions

### retry(func, max_attempts?)

Simple retry without delay between attempts.

```hemlock
let result = retry(fn() {
    // Operation that might fail
    return http_get("https://api.example.com/data");
}, 3);
```

**Parameters:**
- `func`: Function to retry (should throw on failure)
- `max_attempts`: Maximum number of attempts (default: 3)

**Returns:** Result of successful function call
**Throws:** Last error if all attempts fail

### retry_with_backoff(func, options?)

Retry with configurable exponential backoff.

```hemlock
let result = retry_with_backoff(fn() {
    return http_get("https://api.example.com/data");
}, {
    "max_attempts": 5,
    "base_delay": 100,
    "max_delay": 30000,
    "multiplier": 2.0,
    "jitter": true,
    "on_retry": fn(attempt, error, delay) {
        print("Retry " + attempt + " after " + delay + "ms: " + error);
    }
});
```

**Options:**
- `max_attempts`: Maximum attempts (default: 3)
- `base_delay`: Base delay in ms (default: 100)
- `max_delay`: Maximum delay in ms (default: 30000)
- `multiplier`: Backoff multiplier (default: 2.0)
- `jitter`: Add random jitter (default: true)
- `on_retry`: Callback on retry (receives attempt, error, delay)

### retry_linear(func, max_attempts?, delay?)

Retry with constant delay between attempts.

```hemlock
let result = retry_linear(fn() {
    return check_service_status();
}, 5, 2000);  // 5 attempts, 2 seconds between each
```

**Parameters:**
- `func`: Function to retry
- `max_attempts`: Maximum attempts (default: 3)
- `delay`: Delay between attempts in ms (default: 1000)

### retry_if(func, should_retry, max_attempts?, delay?)

Retry only if error matches a condition.

```hemlock
// Only retry on network errors, not validation errors
let result = retry_if(
    fn() { return api_call(); },
    fn(error) { return error.contains("network") || error.contains("timeout"); },
    3,
    1000
);
```

**Parameters:**
- `func`: Function to retry
- `should_retry`: Predicate to check if should retry (receives error)
- `max_attempts`: Maximum attempts (default: 3)
- `delay`: Delay between attempts in ms (default: 1000)

### retry_until(func, condition, options?)

Retry until a condition is met (for polling).

```hemlock
// Poll until job completes
let result = retry_until(
    fn() { return get_job_status(job_id); },
    fn(status) { return status == "completed" || status == "failed"; },
    {
        "max_attempts": 30,
        "delay": 2000,
        "timeout": 60000
    }
);
```

**Options:**
- `max_attempts`: Maximum attempts (default: 10)
- `delay`: Delay between attempts in ms (default: 1000)
- `timeout`: Total timeout in ms (default: null, no timeout)

**Throws:** Error if condition never met or timeout exceeded

## Utility Functions

### with_retry(func, options?)

Create a retry-wrapped version of a function.

```hemlock
let reliable_fetch = with_retry(fn() {
    return http_get("https://api.example.com/data");
}, { "max_attempts": 3 });

// Now reliable_fetch() will automatically retry on failure
let data = reliable_fetch();
```

**Parameters:**
- `func`: Function to wrap
- `options`: Retry options (same as retry_with_backoff)

**Returns:** New function that retries on failure

## Examples

### Basic Retry Pattern

```hemlock
import { retry } from "@stdlib/retry";

let attempts = 0;
let result = retry(fn() {
    attempts = attempts + 1;
    if (attempts < 3) {
        throw "temporary failure";
    }
    return "success";
}, 5);

print("Succeeded after " + attempts + " attempts");
print("Result: " + result);
```

### Network Request with Exponential Backoff

```hemlock
import { retry_with_backoff } from "@stdlib/retry";
import { http_get } from "@stdlib/http";

fn fetch_data(url) {
    return retry_with_backoff(fn() {
        return http_get(url);
    }, {
        "max_attempts": 5,
        "base_delay": 100,
        "multiplier": 2.0,
        "jitter": true,
        "on_retry": fn(attempt, error, delay) {
            print("Request failed, retrying in " + delay + "ms...");
        }
    });
}
```

### Polling for Completion

```hemlock
import { retry_until } from "@stdlib/retry";

fn wait_for_deployment(deploy_id) {
    return retry_until(
        fn() {
            return get_deployment_status(deploy_id);
        },
        fn(status) {
            return status["state"] == "ready" || status["state"] == "failed";
        },
        {
            "max_attempts": 60,
            "delay": 5000,
            "timeout": 300000  // 5 minute timeout
        }
    );
}
```

### Conditional Retry

```hemlock
import { retry_if } from "@stdlib/retry";

fn api_request(endpoint) {
    return retry_if(
        fn() { return http_post(endpoint, data); },
        fn(error) {
            // Only retry on rate limiting or server errors
            return error.contains("429") || error.contains("503");
        },
        5,
        2000
    );
}
```

### Custom Backoff Strategy

```hemlock
import { exponential_backoff, add_jitter } from "@stdlib/retry";
import { sleep } from "@stdlib/time";

fn custom_retry(func, max_attempts) {
    let attempt = 0;
    while (attempt < max_attempts) {
        try {
            let result = func();
            return result;
        } catch (e) {
            if (attempt + 1 >= max_attempts) {
                throw e;
            }

            // Custom backoff: exponential with 25% jitter
            let delay = exponential_backoff(attempt, 200, 60000, 1.5);
            delay = add_jitter(delay, 0.25);

            print("Attempt " + attempt + " failed, retrying in " + delay + "ms");
            sleep(delay);
            attempt = attempt + 1;
        }
    }
}
```

## Best Practices

1. **Always set max_attempts** - Avoid infinite retries
2. **Use jitter for distributed systems** - Prevents thundering herd
3. **Cap max_delay** - Don't wait forever between retries
4. **Log retry attempts** - Use on_retry callback for observability
5. **Only retry transient errors** - Use retry_if for selective retrying
6. **Set timeouts for polling** - Use retry_until with timeout option
