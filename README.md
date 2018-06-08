# annular-queue
It is an annular queue for embedded development. 

# principle
```
+---------------------------------------------------------------------------+
|                                                                           |
|                                                                           |
|   +----+----+----+----+----+----+----+----+----+----+----+----+----+      |
+-> |    |    |    |    |    |    |    |    |    |    |    |    |    | +----+
    |    |    |    |    |    |    |    |    |    |    |    |    |    |
    +----+----+----+----+----+----+----+----+----+----+----+----+----+
     head                     tail

                       head = (head + 1)%length
                       tail = (tail + 1)%length
```

* Open / close the print by the macro definition of ANQUEUE_DEBUG
* Use custom memory by using a macro definition of ANQUEUE_CUSTOM_MEMORY

# example

```c
struct anqueue q;
struct anqueue_param p;
unsigned char buff[100];
int e = 1;
int t;

p.length = 100;
p.elem_size = sizeof(e);
p.mode = ANQUEUE_MODE_COVER;
p.buffer = buff;
p.buffer_size = sizeof(buff);

anqueue_create(&q, &p);

anqueue_push(&q, &e, sizeof(e));
anqueue_front(&q, &t, sizeof(t));
anqueue_pop(&q);

anqueue_delete(&q);
```

