# Volume II. Practical Usage

> **Level:** for developers who have already mastered Volume I.
> **Goal of the volume:** you will learn to build applications on libmdbx — cursors, multivalues and DUPSORT,
> secondary indexes, environment configuration, durability modes, multithreading, and robust
> error handling.
> **End-to-end project:** we grow the application configurator into a full-fledged application with indexes and
> multithreading.
>
> Chapter pattern: concept → mechanism → practice → nuance.

---

## Chapter 6. Cursors

### 6.1. What is a cursor and why do you need one

`mdbx_get` only does point lookups by an exact key. For everything else — traversing a table,
range search, "the nearest greater key", iteration with deletion — you need a **cursor**.

A cursor is a "pointer" to a position inside the tree (more precisely, a stack of positions from the
root to the leaf). It lets you move through the ordered keys in both directions and perform operations
relative to the current position.

### 6.2. Opening and closing

```c
int mdbx_cursor_open(MDBX_txn *txn, MDBX_dbi dbi, MDBX_cursor **cursor);
int mdbx_cursor_close(MDBX_cursor *cursor);
```

A cursor belongs to its transaction: it is created inside it and lives no longer than it. After
`commit`/`abort` the cursor must not be closed — use it only until the transaction ends.

### 6.3. Positioning

```c
int mdbx_cursor_get(MDBX_cursor *cur, MDBX_val *key, MDBX_val *data, MDBX_cursor_op op);
```

Key operations:

| Operation                          | Meaning                                                          |
| ---------------------------------- | --------------------------------------------------------------- |
| `MDBX_FIRST` / `MDBX_LAST`         | Move to the first / last key                                    |
| `MDBX_NEXT` / `MDBX_PREV`          | Next / previous key                                             |
| `MDBX_SET`                         | Find the exact key (position on it)                             |
| `MDBX_SET_RANGE`                   | Find the first key ≥ the given one ("nearest greater or equal") |
| `MDBX_SET_LOWERBOUND`              | Like `SET_RANGE`, but requires a valid data argument for DUPSORT |
| `MDBX_SET_UPPERBOUND`              | Find the first key > the given one                              |
| `MDBX_GET_BOTH` / `GET_BOTH_RANGE` | (DUPSORT) find a specific value / the first ≥ the value         |

After `MDBX_LAST`, when the data runs out, the operation returns `MDBX_NOTFOUND`, and the cursor
lands "at the end" — the `eof` state. This is a normal situation for finishing a traversal.

For frequent scenarios there are dedicated convenience functions instead of a "`cursor_get(op)` +
code check" pair: `mdbx_cursor_on_first()`, `mdbx_cursor_on_last()` and their dup variants
`mdbx_cursor_on_first_dup()`/`mdbx_cursor_on_last_dup()`; to test "end of traversal" — the logical
`mdbx_cursor_eof()`. The distance between two cursor positions can be found via
`mdbx_cursor_distance()` — useful to size up a range before processing it.

**Fragment from [`examples/c++/05-cursors.c++`](examples/c++/05-cursors.c++)** — forward iteration (FIRST/NEXT) and `SET_RANGE` ("nearest greater or equal"):

```cpp
    auto cur = rtxn.open_cursor(table);

    // Forward iteration: FIRST then NEXT until the end.
    size_t forward_count = 0;
    std::string forward;
    for (auto r = cur.to_first(); r; r = cur.to_next(false)) {
      forward += r.key.as_string() + " ";
      ++forward_count;
    }
    std::cout << "forward (" << forward_count << "): " << forward << "\n";

    // SET_RANGE: the first key not less than the given one.
    auto r = cur.to_key_greater_or_equal(mdbx::slice("k5"), false);
    std::cout << "set_range(k5) -> ";
    if (r)
      std::cout << r.key.as_string() << "\n";
    else
      std::cout << "<none>\n";
    r = cur.to_key_greater_or_equal(mdbx::slice("k5x"), false);
    std::cout << "set_range(k5x) -> ";
    if (r)
      std::cout << r.key.as_string() << "\n";
    else
      std::cout << "<none>\n";
```

Full code: [05-cursors.c++](examples/c++/05-cursors.c++) · [C version](examples/c/05-cursors.c)

### 6.4. Example: table traversal and range search

```c
#include <stdio.h>
#include <string.h>
#include <mdbx.h>

static void die(const char *w, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", w, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL; MDBX_txn *txn = NULL; MDBX_dbi dbi;
    MDBX_cursor *cur = NULL;
    MDBX_val k, d;
    int rc;

    rc = mdbx_env_create(&env); if (rc) die("create", rc);
    rc = mdbx_env_open(env, "./cur.mdbx", MDBX_NOSUBDIR, 0664);
    if (rc) die("open", rc);
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn", rc);
    rc = mdbx_dbi_open(txn, "kv", MDBX_CREATE, &dbi);
    if (rc) die("dbi", rc);

    /* fill the table */
    const char *words[] = {"alpha","bravo","charlie","delta","echo"};
    for (int i = 0; i < 5; i++) {
        k.iov_base = (void *)words[i]; k.iov_len = strlen(words[i]);
        d.iov_base = (void *)"x"; d.iov_len = 1;
        rc = mdbx_put(txn, dbi, &k, &d, 0); if (rc) die("put", rc);
    }

    /* full traversal */
    rc = mdbx_cursor_open(txn, dbi, &cur); if (rc) die("cursor", rc);
    printf("All keys:\n");
    while ((rc = mdbx_cursor_get(cur, &k, &d, MDBX_NEXT)) == MDBX_SUCCESS)
        printf("  %.*s\n", (int)k.iov_len, (char *)k.iov_base);
    /* at the end MDBX_NOTFOUND is returned — this is normal */
    if (rc != MDBX_NOTFOUND) die("next", rc);

    /* range: the first key >= "c" and all the following */
    printf("Range >= c:\n");
    const char *from = "c";
    k.iov_base = (void *)from; k.iov_len = strlen(from);
    rc = mdbx_cursor_get(cur, &k, &d, MDBX_SET_RANGE);
    if (rc == MDBX_SUCCESS) {
        do {
            printf("  %.*s\n", (int)k.iov_len, (char *)k.iov_base);
        } while ((rc = mdbx_cursor_get(cur, &k, &d, MDBX_NEXT)) == MDBX_SUCCESS);
    }
    if (rc != MDBX_NOTFOUND) die("range", rc);

    mdbx_cursor_close(cur);
    mdbx_txn_commit(txn);
    mdbx_env_close(env);
    return 0;
}
```

### 6.5. Cursor after deletion — UB (critical warning)

> **Warning:** after `mdbx_cursor_del()` the cursor position is undefined. Using the cursor
> without re-positioning is undefined behavior. Always do `NEXT` (or `PREV`) after deletion and check
> `MDBX_NOTFOUND`.

### 6.6. Cloning cursors

`mdbx_cursor_clone()` duplicates the cursor position (for parallel processing on a single snapshot),
`mdbx_cursor_bind()` binds an existing cursor to another transaction/table.

### 6.7. The safe-delete pattern in DUPSORT

Deleting values in a DUPSORT table with a single cursor during iteration is dangerous (known bugs in
0.12.x, details in Volume V). The safe pattern is **two cursors**: one positions, the other deletes.

**Fragment (C, illustration):** the full compilable version is below and in
[`examples/c++/06-dupsort-delete.c++`](examples/c++/06-dupsort-delete.c++).

```c
/* delete all values of the "target" key in a DUPSORT table */
MDBX_cursor *it, *del;
mdbx_cursor_open(txn, dbi, &it);
mdbx_cursor_open(txn, dbi, &del);
const char *tgt = "target";
k.iov_base = (void *)tgt; k.iov_len = strlen(tgt);
rc = mdbx_cursor_get(it, &k, &d, MDBX_SET);          /* first dup */
while (rc == MDBX_SUCCESS) {
    MDBX_val dk = k, dv = d;
    /* second cursor on the same position */
    rc = mdbx_cursor_get(del, &dk, &dv, MDBX_GET_BOTH);
    if (rc != MDBX_SUCCESS) break;
    rc = mdbx_cursor_del(del, MDBX_NODUPDATA);
    if (rc) break;
    rc = mdbx_cursor_get(it, &k, &d, MDBX_NEXT_DUP);
}
mdbx_cursor_close(it);
mdbx_cursor_close(del);
```

For bulk deletion of whole ranges, better use `mdbx_cursor_bunch_delete()`
("bunch deletion") — it cuts out whole pages and branches rather than iterating elements.

**Fragment from [`examples/c++/06-dupsort-delete.c++`](examples/c++/06-dupsort-delete.c++)** — the safe-delete pattern with two cursors: `it` iterates over duplicates (`NEXT_DUP`), `del` positions via `GET_BOTH` and deletes the current value:

```cpp
    auto txn = env.start_write();
    auto multi = txn.open_map("multi", mdbx::key_mode::usual, mdbx::value_mode::multi);
    dump_key(txn, multi, "values before delete (target):", mdbx::slice("target"));

    // The "two cursors" pattern: `it` iterates over the key's duplicates,
    // `del` deletes the current pair (the `it` position stays valid).
    const mdbx::slice target("target");
    auto it = txn.open_cursor(multi);
    auto del = txn.open_cursor(multi);
    size_t deleted = 0;
    // Keep the first dup of the key, delete the rest.
    auto pos = it.to_key_exact(target);
    if (pos)
      pos = it.to_current_next_multi(false);
    for (; pos; pos = it.to_current_next_multi(false)) {
      del.to_exact_key_value_equal(pos.key, pos.value, false); // MDBX_GET_BOTH
      if (del.erase(false)) // MDBX_CURRENT: delete only the current value
        ++deleted;
    }
```

Full code: [06-dupsort-delete.c++](examples/c++/06-dupsort-delete.c++) · [C version](examples/c/06-dupsort-delete.c)

> **Examples for this chapter:** [`examples/c++/05-cursors.c++`](examples/c++/05-cursors.c++) · [C version](examples/c/05-cursors.c);
> [`examples/c++/06-dupsort-delete.c++`](examples/c++/06-dupsort-delete.c++) · [C version](examples/c/06-dupsort-delete.c);
> end-to-end project: [`config-store-06.c++`](examples/config-store/config-store-06.c++).

### 6.8. Summary of chapter 6

- A cursor is a position in the ordered tree; `mdbx_cursor_get(op)` controls movement.
- `MDBX_SET_RANGE` is the foundation of range queries.
- After `cursor_del` the position is invalid — re-position.
- For deletion in DUPSORT — two cursors or `bunch_delete`.

### 6.9. Exercises

1. Write a function that walks all keys from end to start (`MDBX_LAST` + `MDBX_PREV`).
2. Find "the key after X" in two ways: `SET_RANGE` and `SET_UPPERBOUND`.
3. Delete every second key during a traversal and explain why the positioning must be refreshed.

### 6.10. Chapter 6 checklist

- [ ] I can open and close a cursor and understand that it lives no longer than its transaction;
- [ ] I use `MDBX_FIRST`/`MDBX_LAST`/`MDBX_NEXT`/`MDBX_PREV` and `MDBX_SET_RANGE` fluently for range queries;
- [ ] I remember that the position is undefined after `mdbx_cursor_del()` and always re-position;
- [ ] I can apply the "two cursors" pattern for safe deletion in DUPSORT;
- [ ] I can walk a table from end to start (`MDBX_LAST` + `MDBX_PREV`) and tell end-of-traversal (`MDBX_NOTFOUND`, `mdbx_cursor_eof()`) from an error.

### 6.11. What's next

The next chapter extends the data model: the `MDBX_DUPSORT` flag turns a table into a multimap
"key → ordered set of values". We will cover the storage forms, the `MDBX_DUPFIXED`,
`MDBX_INTEGERDUP`, `MDBX_REVERSEDUP` flags, navigation over duplicates, and the key technique —
the inverted index "field → list of IDs". It becomes the basis of the configurator's secondary
indexes in chapter 8.

---

## Chapter 7. Multivalues and DUPSORT

### 7.1. What is DUPSORT

A regular table stores one key → one value. The `MDBX_DUPSORT` flag turns the table into a
**multimap**: one key → **an ordered set of values**. The value plays the role of a "second key"
with its own sort order.

Example: user tags `user:1001 → {"admin", "staff", "vip"}`.

### 7.2. Storage forms

Internally (details in Volume III) the representation of values is chosen by their count:

- a few values per key — a dense **sub-page** (a nested page inside the leaf);
- many values — a separate **nested B+tree**;
- fixed-size values — specialized **dupfix pages** with dense packing.

An important consequence: a "multivalue" never goes to overflow pages, so it is limited by the
key's frame (about half a page).

### 7.3. DUPSORT flags

| Flag              | Meaning                                                                                |
| ----------------- | -------------------------------------------------------------------------------------- |
| `MDBX_DUPSORT`    | The table is a multimap                                                                 |
| `MDBX_DUPFIXED`   | All values have a fixed length (requires equal lengths! otherwise `MDBX_BAD_VALSIZE`)  |
| `MDBX_INTEGERDUP` | Values are `uint32_t`/`uint64_t` in native order (requires `DUPFIXED`+`DUPSORT`)        |
| `MDBX_REVERSEDUP` | Reverse sort order of values                                                            |

### 7.4. Operations with multivalues

```c
/* add a value to a key (MDBX_NODUPDATA protects against duplicates) */
mdbx_put(txn, dbi, &key, &data, MDBX_NODUPDATA);

/* find a specific value */
mdbx_cursor_get(cur, &key, &data, MDBX_GET_BOTH);       /* exactly */
mdbx_cursor_get(cur, &key, &data, MDBX_GET_BOTH_RANGE); /* the first >= data */

/* navigation over the key's values */
mdbx_cursor_get(cur, &key, &data, MDBX_FIRST_DUP);
mdbx_cursor_get(cur, &key, &data, MDBX_LAST_DUP);
mdbx_cursor_get(cur, &key, &data, MDBX_NEXT_DUP);
mdbx_cursor_get(cur, &key, &data, MDBX_PREV_DUP);

/* number of the key's values */
size_t count;
mdbx_cursor_count(cur, &count);
```

### 7.5. Inverted indexes: the "key → list of IDs" pattern

The classic use of DUPSORT is an **inverted index**: field value → list of record identifiers.
This is a "cheap secondary index" without a separate link table.

**Fragment (C, illustration):** the full compilable version is below and in
[`examples/c++/08-inverted-index.c++`](examples/c++/08-inverted-index.c++).

```c
/* index "role" -> list of user_id */
MDBX_dbi idx_role;
mdbx_dbi_open(txn, "idx_role", MDBX_CREATE | MDBX_DUPSORT, &idx_role);

/* when creating a user: role = "admin", id = 1001 */
MDBX_val k = { (void *)"admin", 5 };
MDBX_val v = { &user_id_u64, sizeof(user_id_u64) };
mdbx_put(txn, idx_role, &k, &v, MDBX_NODUPDATA);
```

Getting the list of admins is a cursor over `idx_role` with the key `"admin"` and `NEXT_DUP`.

**Fragment from [`examples/c++/08-inverted-index.c++`](examples/c++/08-inverted-index.c++)** — building an inverted index "word → list of document IDs" over a DUPSORT table (`value_mode::multi`):

```cpp
    {
      auto txn = env.start_write();
      auto docs = txn.create_map("docs", mdbx::key_mode::usual, mdbx::value_mode::single);
      // Index: key — a word, values — document IDs (multivalues).
      auto words = txn.create_map("words", mdbx::key_mode::usual, mdbx::value_mode::multi);

      static const struct {
        const char *id;
        const char *text;
      } entries[] = {{"doc0", "the quick brown fox"}, {"doc1", "quick red fox"}, {"doc2", "lazy brown dog"}};

      for (const auto &e : entries) {
        txn.insert(docs, mdbx::slice(e.id), mdbx::slice(e.text));
        std::istringstream stream(e.text);
        std::string word;
        while (stream >> word)
          txn.upsert(words, mdbx::slice(word), mdbx::slice(e.id)); // UPSERT appends the ID to the word
      }
      txn.commit();
    }
```

Full code: [08-inverted-index.c++](examples/c++/08-inverted-index.c++) · [C version](examples/c/08-inverted-index.c)

### 7.6. Full example: a secondary index on DUPSORT

```c
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <mdbx.h>

static void die(const char *w, int rc) {
    fprintf(stderr, "%s: %s (%d)\n", w, mdbx_strerror(rc), rc);
    exit(1);
}

int main(void) {
    MDBX_env *env = NULL; MDBX_txn *txn = NULL;
    MDBX_dbi users, by_role;
    MDBX_val k, v, d;
    MDBX_cursor *cur = NULL;
    int rc;

    rc = mdbx_env_create(&env); if (rc) die("create", rc);
    rc = mdbx_env_open(env, "./idx.mdbx", MDBX_NOSUBDIR, 0664);
    if (rc) die("open", rc);
    rc = mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) die("txn", rc);

    mdbx_dbi_open(txn, "users", MDBX_CREATE | MDBX_INTEGERKEY, &users);
    mdbx_dbi_open(txn, "by_role", MDBX_CREATE | MDBX_DUPSORT, &by_role);

    /* user_id -> name;  by_role["admin"] -> {id,...} */
    struct { uint64_t id; const char *name; const char *role; } rows[] = {
        {1001, "Anna",   "admin"}, {1002, "Boris", "user"},
        {1003, "Victor", "admin"}, {1004, "Galina", "user"},
    };
    for (int i = 0; i < 4; i++) {
        k.iov_base = &rows[i].id; k.iov_len = sizeof(uint64_t);
        v.iov_base = (void *)rows[i].name; v.iov_len = strlen(rows[i].name);
        mdbx_put(txn, users, &k, &v, 0);

        k.iov_base = (void *)rows[i].role; k.iov_len = strlen(rows[i].role);
        v.iov_base = &rows[i].id; v.iov_len = sizeof(uint64_t);
        mdbx_put(txn, by_role, &k, &v, MDBX_NODUPDATA);
    }

    /* print all admins via the index */
    const char *role = "admin";
    k.iov_base = (void *)role; k.iov_len = strlen(role);
    mdbx_cursor_open(txn, by_role, &cur);
    rc = mdbx_cursor_get(cur, &k, &d, MDBX_SET);
    if (rc == MDBX_SUCCESS) {
        printf("Admins:\n");
        do {
            printf("  user id = %llu\n",
                   (unsigned long long)*(uint64_t *)d.iov_base);
        } while ((rc = mdbx_cursor_get(cur, &k, &d, MDBX_NEXT_DUP)) == MDBX_SUCCESS);
    }
    mdbx_cursor_close(cur);

    mdbx_txn_commit(txn);
    mdbx_env_close(env);
    return 0;
}
```

> **Examples for this chapter:** [`examples/c++/07-dupsort.c++`](examples/c++/07-dupsort.c++) · [C version](examples/c/07-dupsort.c);
> [`examples/c++/08-inverted-index.c++`](examples/c++/08-inverted-index.c++) · [C version](examples/c/08-inverted-index.c).

### 7.7. Summary of chapter 7

- `MDBX_DUPSORT` = key → ordered set of values.
- `DUPFIXED` requires equal lengths; `INTEGERDUP` — integer values in native order.
- Navigation over values: `FIRST_DUP/LAST_DUP/NEXT_DUP/PREV_DUP/GET_BOTH`.
- An inverted index on DUPSORT is a cheap secondary index.

### 7.8. Exercises

1. Build an index "by age" and print all users older than 30.
2. Explain why `MDBX_NODUPDATA` is needed when adding to a DUPSORT index.
3. What happens on an attempt to `DUPFIXED`-insert a value of a different length?

### 7.9. Chapter 7 checklist

- [ ] I understand that `MDBX_DUPSORT` means "key → ordered set of values", with the value acting as a second key;
- [ ] I know when `MDBX_DUPFIXED` (equal value lengths) and `MDBX_INTEGERDUP` are required;
- [ ] I can locate and iterate the values of a key: `MDBX_GET_BOTH`, `MDBX_GET_BOTH_RANGE`, `MDBX_FIRST_DUP`/`NEXT_DUP`, `mdbx_cursor_count()`;
- [ ] I add values with `MDBX_NODUPDATA` to avoid duplicates;
- [ ] I can build an inverted index "word → list of IDs" over a DUPSORT table.

### 7.10. What's next

Multivalues are a half-finished product; chapter 8 assembles them into a complete technique: the
secondary index. We will build the "main table + index tables" schema, learn to keep them
consistent in a single transaction, compare a DUPSORT index with a separate table, and meet
composite keys and deletion via an index. For the end-to-end project this is a step toward a
"mini-ORM" with a users entity.

---

## Chapter 8. Secondary indexes

### 8.1. Why secondary indexes are needed in a KV database

There are no SQL indexes in a key-value database: data is searched only by the primary key. To search
"by another field", we build the index **ourselves** — a separate table whose key is the sought field
and whose value is the primary key of the original record.

### 8.2. Pattern: main table + index tables

```
users:        user_id -> {name, email, ...}        (primary)
idx_email:    email   -> user_id                    (index)
idx_by_role:  role    -> user_id  (DUPSORT)         (index)
```

Keeping consistency is **on your side**: on every change to `users` update all indexes in the
**same transaction** (otherwise there will be drift).

**Fragment from [`examples/c++/09-secondary-index.c++`](examples/c++/09-secondary-index.c++)** — the main table and the index are updated in one transaction: on user insertion — and on deletion (the record + its index references):

```cpp
    {
      auto txn = env.start_write();
      auto users = txn.create_map("users", mdbx::key_mode::usual, mdbx::value_mode::single);
      // Index: key — a role, values — user IDs (multivalues).
      auto by_role = txn.create_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);

      // Inserting users and maintaining the index — in one transaction.
      txn.insert(users, mdbx::slice("user1"), mdbx::slice("Alice|admin"));
      txn.upsert(by_role, mdbx::slice("admin"), mdbx::slice("user1"));
      txn.insert(users, mdbx::slice("user2"), mdbx::slice("Bob|dev"));
      txn.upsert(by_role, mdbx::slice("dev"), mdbx::slice("user2"));
      txn.insert(users, mdbx::slice("user3"), mdbx::slice("Carol|admin"));
      txn.upsert(by_role, mdbx::slice("admin"), mdbx::slice("user3"));
      txn.commit();
    }

    // Deleting a user together with their index entry.
    {
      auto txn = env.start_write();
      auto users = txn.open_map("users", mdbx::key_mode::usual, mdbx::value_mode::single);
      auto by_role = txn.open_map("by_role", mdbx::key_mode::usual, mdbx::value_mode::multi);
      txn.erase(users, mdbx::slice("user2"));
      txn.erase(by_role, mdbx::slice("dev"), mdbx::slice("user2")); // a specific key's value
      txn.commit();
    }
```

Full code: [09-secondary-index.c++](examples/c++/09-secondary-index.c++)

> **Nuance: opening a table with unknown flags.** If a table may have been created by other code
> (and its persistent flags — `MDBX_DUPSORT`, `MDBX_INTEGERKEY`, etc. — are unknown in advance),
> pass `MDBX_DB_ACCEDE` when opening it: instead of `MDBX_INCOMPATIBLE` the table opens with its
> actual flags. You can discover them via `mdbx_dbi_flags()` (the exact variant —
> `mdbx_dbi_flags_ex()` — additionally returns the table state bits:
> `MDBX_DBI_CREAT`/`MDBX_DBI_DIRTY`/`MDBX_DBI_FRESH`/`MDBX_DBI_STALE`). The same applies to reopening tables
> in a new transaction: an `MDBX_dbi` handle from an old transaction does not carry over, and
> `mdbx_dbi_open()` in a new transaction without the same flags returns `MDBX_INCOMPATIBLE`.

### 8.3. DUPSORT index vs a separate table

- **DUPSORT index** ("field → list of IDs") is ideal for "one-to-many": the recipient list is the
  set of values of one key. Fast add/remove of a single ID.
- **Separate table** ("composite key → marker") — when you need more data about the relationship or
  composite conditions.

### 8.4. Composite indexes

A composite key is a concatenation of fields in a single key. Byte order is critical: for a numeric
field inside a composite key to compare correctly, it must be converted to **big-endian** order (or
use integer types of native order with a separate comparator).

**Fragment from [`examples/c++/10-composite-key.c++`](examples/c++/10-composite-key.c++)** — packing two fields into a composite key with big-endian byte order: the lexicographic order of the key matches the numeric order of the pair:

```cpp
// Packing (x, y) into a uint64_t so that the byte-wise (lexicographic) order
// of the key matches the numeric order of the pair. For that, a big-endian
// representation of unsigned fixed-width fields is used.
uint64_t pack(uint32_t x, uint32_t y) {
  const uint64_t value = (uint64_t(x) << 32) | uint64_t(y);
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return __builtin_bswap64(value);
#else
  return value;
#endif
}
```

Full code: [10-composite-key.c++](examples/c++/10-composite-key.c++)

### 8.5. Deleting by index

You cannot delete via an index directly — find the primary key via the index, then delete the record
and update the indexes. Use the safe iteration pattern (chapter 6).

### 8.6. Mini-ORM over libmdbx (end-to-end project)

We extend the configurator: add a "users" entity with an index by name.

```c
/* schema: users(id -> name), idx_name(name -> id) */
int user_add(config_store_t *cs, uint64_t id, const char *name) {
    MDBX_txn *txn; MDBX_dbi users, idx;
    MDBX_val k, v;
    int rc;

    rc = mdbx_txn_begin(cs->env, NULL, MDBX_TXN_READWRITE, &txn);
    if (rc) return rc;
    mdbx_dbi_open(txn, "users", MDBX_CREATE | MDBX_INTEGERKEY, &users);
    mdbx_dbi_open(txn, "idx_name", MDBX_CREATE | MDBX_DUPSORT, &idx);

    k.iov_base = &id; k.iov_len = sizeof(id);
    v.iov_base = (void *)name; v.iov_len = strlen(name);
    rc = mdbx_put(txn, users, &k, &v, MDBX_NOOVERWRITE);
    if (rc == MDBX_KEYEXIST) { mdbx_txn_abort(txn); return rc; }

    /* index: name -> id */
    k.iov_base = (void *)name; k.iov_len = strlen(name);
    v.iov_base = &id; v.iov_len = sizeof(id);
    rc = mdbx_put(txn, idx, &k, &v, MDBX_NODUPDATA);

    rc = mdbx_txn_commit(txn);
    return rc;
}
```

> **Nuance:** atomicity guarantees exactly "all or nothing": if a crash happens between
> operations, the transaction rolls back entirely. This is the price of correct indexes — update them
> only in one transaction together with the main record.

> **Examples for this chapter:** [`examples/c++/09-secondary-index.c++`](examples/c++/09-secondary-index.c++);
> [`examples/c++/10-composite-key.c++`](examples/c++/10-composite-key.c++);
> end-to-end project: [`config-store-08.c++`](examples/config-store/config-store-08.c++).

### 8.7. Summary of chapter 8

- A secondary index is a "field → primary key" table that you maintain yourself.
- All changes (data + indexes) go in one transaction.
- A DUPSORT index for "one-to-many"; composite keys for several fields.
- Delete a record together with its index entries; otherwise the index goes stale.
- An inverted index over DUPSORT is a cheap alternative to a separate link table.

### 8.8. Exercises

1. Add an index "by city" and write a lookup function.
2. Write `user_rename`, which atomically changes the name and rebuilds `idx_name`.
3. Why do composite keys require care with byte order?

### 8.9. Chapter 8 checklist

- [ ] I understand that a secondary index is a "field → primary key" table that I maintain myself;
- [ ] I update the data and all indexes in one transaction, remembering the price of breaking this rule;
- [ ] I choose deliberately between a DUPSORT index ("field → list of IDs") and a separate table;
- [ ] I know why numeric fields in composite keys are packed big-endian;
- [ ] I delete a record together with its index entries; otherwise the index goes stale;
- [ ] I remember `MDBX_DB_ACCEDE` for the case when a table's flags are unknown in advance.

### 8.10. What's next

Indexes and data lead to the next question: how to configure the environment itself. Chapter 9 is
about file geometry (`mdbx_env_set_geometry()`), the `maxreaders`/`maxdbs` limits, environment
flags, `MDBX_opt_*` runtime options, and statistics. A configurator that grows from a test database
to a working one needs a managed file size and predictable limits — otherwise `MDBX_MAP_FULL`
arrives sooner or later.

---

## Chapter 9. Environment configuration

### 9.1. File geometry

Geometry determines how the database file grows and shrinks:

```c
int mdbx_env_set_geometry(MDBX_env *env,
    intptr_t size_lower, intptr_t size_now, intptr_t size_upper,
    intptr_t growth_step, intptr_t shrink_threshold, unsigned pagesize);
```

Parameters (`-1` = "default/do not change"):

| Parameter          | Meaning                                                                |
| ------------------ | ---------------------------------------------------------------------- |
| `size_lower`       | Lower bound of the file size (do not shrink below)                     |
| `size_now`         | Initial size at creation                                                |
| `size_upper`       | **Hard limit**; on reaching it with no space left — `MDBX_MAP_FULL`    |
| `growth_step`      | File growth step                                                        |
| `shrink_threshold` | Threshold at which the file may be truncated                            |
| `pagesize`         | Page size (256…65536; set before the first open)                        |

> **Warning:** set geometry **once, before `env_open`** (or at creation). Changing `upper`
> "on the fly" is not reliably possible. The engine picks the `upper` value for a new database itself
> (~golden section of RAM, bounded by the mmap limit ≈140 TB on 64-bit); `TOO_LARGE`/`ENOMEM` are
> possible with an obviously excessive `upper` (e.g. under ASAN/Valgrind) — always set a reasonable
> `upper` explicitly.

> **Historical note.** `mdbx_env_set_mapsize(env, size)` is a legacy wrapper over
> `mdbx_env_set_geometry()`, equivalent to `mdbx_env_set_geometry(env, size, size, size, -1, -1, -1)`.
> Like geometry, it resizes the database "on the fly" (including after `mdbx_env_open()`), but pins
> the lower/current/upper bounds to a single value — the database loses the ability to auto-grow above
> `size`. Prefer `mdbx_env_set_geometry()` with separate parameters. Likewise `mdbx_env_sync()` is a
> legacy synonym for `mdbx_env_sync_ex(env, force=true, nonblock=false)` — a blocking full
> synchronization (chapter 10).

### 9.2. maxreaders / maxdbs / pagesize

```c
mdbx_env_set_maxreaders(env, readers);  /* reader-table slots, before open */
mdbx_env_set_maxdbs(env, dbs);          /* limit of named tables */
/* the page size is set via the pagesize argument of mdbx_env_set_geometry() — there is no separate function */
```

Current values are read back with the getter pairs `mdbx_env_get_maxreaders()`/`mdbx_env_get_maxdbs()`.

`maxreaders` — how many threads can hold read transactions at the same time. Do not set it too low in
applications with thread pools.

### 9.3. Environment flags

```c
mdbx_env_set_flags(env, flags, onoff);
mdbx_env_get_flags(env, &flags);
```

Key flags: `MDBX_RDONLY`, `MDBX_WRITEMAP`, `MDBX_NOMETASYNC`, `MDBX_SAFE_NOSYNC`,
`MDBX_UTTERLY_NOSYNC`, `MDBX_NOSTICKYTHREADS`, `MDBX_EXCLUSIVE`.

Two more important flags are passed **at `mdbx_env_open()`**, not via `set_flags`:

- `MDBX_ACCEDE` — open a database **already used by another process in an unknown mode**: instead of
  an `MDBX_INCOMPATIBLE` error the environment opens in a mode compatible with the current usage
  (applies to the durability flags, `MDBX_LIFORECLAIM` and `MDBX_NORDAHEAD`). Has no effect if the
  current process is the only one or all opens are read-only.
- `MDBX_EXCLUSIVE` — exclusive open: succeeds only if the database is not opened by anyone else.

A couple of useful environment functions: `mdbx_env_warmup()` — "warm up" the database file into RAM
(the page cache) before starting work; `mdbx_env_get_fd()` — the data file descriptor (for your own
fsync/backup purposes).

### 9.4. Runtime options (MDBX_opt_*)

Fine tuning via `mdbx_env_set_option`/`mdbx_env_get_option`:

| Option                                       | Meaning                                                                                       |
| -------------------------------------------- | --------------------------------------------------------------------------------------------- |
| `MDBX_opt_rp_augment_limit`                  | Limit of list accumulation when searching sequences in the GC                                 |
| `MDBX_opt_gc_time_limit`                     | Time limit for GC search inside a write transaction (1/65536 s)                               |
| `MDBX_opt_txn_dp_limit`                      | Dirty-page limit of a transaction (by default ≈1/42 of RAM)                                   |
| `MDBX_opt_loose_limit`                       | Loose-pages cache (default 64)                                                                |
| `MDBX_opt_writethrough_threshold`            | Threshold for choosing `O_DSYNC` vs `fdatasync` (default 2 dirty pages; ignored on Windows)   |
| `MDBX_opt_prefault_write_enable`             | Pre-fault writing for WRITEMAP                                                                |
| `MDBX_opt_sync_bytes` / `MDBX_opt_sync_period` | Auto-synchronization (defaults: ~1.05 GB / 42.42 s; explicit 0 = disabled)                     |
| `MDBX_opt_merge_threshold`                   | Page merge threshold (16.16 format; default 33%, range [12.5%..50%])                          |
| `MDBX_opt_prefer_waf_insteadof_balance`      | Prefer the dirty neighbor when merging (default true since 2026-01-04)                        |

**Fragment from [`examples/c++/11-geometry-options.c++`](examples/c++/11-geometry-options.c++)** — setting geometry before `open` (via `create_parameters().set_geometry(geo)`) and runtime options after:

```cpp
    // Geometry: lower/current/upper bounds of the DB size, the growth step,
    // and the shrink threshold. Here — a compact database for the example.
    mdbx::env::geometry geo;
    geo.size_lower = 8 * mdbx::env::geometry::MB;
    geo.size_now = 16 * mdbx::env::geometry::MB;
    geo.size_upper = 64 * mdbx::env::geometry::MB;
    geo.growth_step = 8 * mdbx::env::geometry::MB;
    geo.shrink_threshold = 2 * mdbx::env::geometry::MB;

    mdbx::env_managed env(path, mdbx::env_managed::create_parameters().set_geometry(geo),
                          mdbx::env::operate_parameters());

    // Runtime options: set and read back.
    env.set_extra_option(mdbx::env::extra_runtime_option::writethrough_threshold, 1 << 20);
    env.set_extra_option(mdbx::env::extra_runtime_option::merge_threshold_dot16, 65536 / 3);
    env.set_sync_threshold(256 * 1024); // sync_bytes = 256 KiB
    env.set_extra_option(mdbx::env::extra_runtime_option::prefault_write_enable, 1);
```

Full code: [11-geometry-options.c++](examples/c++/11-geometry-options.c++) · [C version](examples/c/11-geometry-options.c)

### 9.5. Choosing the page size

| Page   | When                                                                |
| ------ | ------------------------------------------------------------------- |
| 4 KB   | Default; universal                                                  |
| 8 KB   | Insurance against a large fragmented GC                             |
| 64 KB  | Large values (fewer overflow trips); more expensive small writes    |

> Note: a value that does not fit into a single page is stored as one unbroken
> piece on consecutive pages and needs a run of **consecutive** free pages —
> allocating it is costlier and, under fragmentation, may end in
> `MDBX_MAP_FULL`. So when choosing the page size, keep the actual (typical and
> maximum) value sizes in mind: the larger the share of values that do not fit
> into one page, the more attractive a large page. If a long value cannot be
> made smaller, split it into chunks that fit into one page
> (`mdbx_limits_valsize4page_max()`); manual splitting is needed only until the
> planned **streamed API for long values** ("Streaming BLOBs") arrives. Details —
> [Restrictions & gotchas](https://libmdbx.dqdkfa.ru/docs/en/overview/restrictions.html).

### 9.6. Statistics

```c
mdbx_env_info_ex(env, &info, sizeof(info));  /* geometry, metas, counters */
mdbx_env_stat_ex(env, txn, &stat, sizeof(stat)); /* sizes, pages */
mdbx_dbi_stat_ex(txn, dbi, &dst, sizeof(dst), 0); /* table statistics */
```

**Fragment from [`examples/c++/12-env-stat.c++`](examples/c++/12-env-stat.c++)** — reading environment statistics (the analogues of `mdbx_env_stat_ex()`/`mdbx_env_info_ex()`):

```cpp
    {
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      for (int i = 0; i < 100; ++i) {
        const auto key = std::to_string(i);
        const auto value = "value-" + std::to_string(i);
        txn.insert(table, mdbx::slice(key), mdbx::slice(value));
      }
      txn.commit();
    }

    const auto stat = env.get_stat(); // analogue of mdbx_env_stat_ex()
    std::cout << "stat: ps=" << stat.ms_psize << " depth=" << stat.ms_depth
              << " leaf=" << stat.ms_leaf_pages << " branch=" << stat.ms_branch_pages
              << " overflow=" << stat.ms_overflow_pages << " entries=" << stat.ms_entries << "\n";

    const auto info = env.get_info(); // analogue of mdbx_env_info_ex()
    std::cout << "info: recent_txnid=" << info.mi_recent_txnid
              << " latter_reader_txnid=" << info.mi_latter_reader_txnid << " geo.current=" << info.mi_geo.current
              << "\n";
```

Full code: [12-env-stat.c++](examples/c++/12-env-stat.c++) · [C version](examples/c/12-env-stat.c)

> **Examples for this chapter:** [`examples/c++/11-geometry-options.c++`](examples/c++/11-geometry-options.c++) · [C version](examples/c/11-geometry-options.c);
> [`examples/c++/12-env-stat.c++`](examples/c++/12-env-stat.c++) · [C version](examples/c/12-env-stat.c);
> end-to-end project: [`config-store-09.c++`](examples/config-store/config-store-09.c++).

### 9.7. Summary of chapter 9

- Geometry (lower/now/upper/growth/shrink/pagesize) is set before open.
- `upper` — a hard limit; too low → `MDBX_MAP_FULL`, obviously excessive (e.g. under
  ASAN/Valgrind) → `TOO_LARGE`/`ENOMEM`. The engine picks the default for a new database itself (≈ golden section of RAM).
- `maxreaders`/`maxdbs`/`pagesize` — before the first open.
- Options `MDBX_opt_*` — fine tuning of GC/spill/synchronization.

### 9.8. Exercises

1. Set the geometry "from 1 MB to 4 GB, step 64 MB" before open and check `env_info`.
2. What does `env_open` return with an obviously excessive `upper` (e.g. 140 TB on 64-bit)? Record the error code.
3. Create a database with a 64 KB page and compare with 4 KB when reading large values.

### 9.9. Chapter 9 checklist

- [ ] I set the geometry (`size_lower`/`size_now`/`size_upper`/`growth_step`/`shrink_threshold`) before `env_open`;
- [ ] I understand that `size_upper` is a hard limit: exhausting it yields `MDBX_MAP_FULL`, while an excessive `upper` yields `TOO_LARGE`/`ENOMEM`;
- [ ] I know where `maxreaders`, `maxdbs`, and the page size are set;
- [ ] I distinguish the key environment flags (`MDBX_WRITEMAP`, `MDBX_SAFE_NOSYNC`, `MDBX_EXCLUSIVE`, `MDBX_ACCEDE`) and the main `MDBX_opt_*` options;
- [ ] I can take statistics via `mdbx_env_info_ex()`/`mdbx_env_stat_ex()` and use it for diagnostics.

### 9.10. What's next

A configured environment is not the whole story: you must also decide what happens on commit.
Chapter 10 covers durability modes: weak/steady meta, what `MDBX_NOMETASYNC` and `MDBX_SAFE_NOSYNC`
risk, why auto-synchronization is needed, and how the modes differ on macOS, Linux, and Windows.
The mode choice directly determines the configurator's speed and reliability.

---

## Chapter 10. Durability modes

### 10.1. What "committing" means

A strict commit must: (1) write data to disk (new page versions) and (2) write the meta —
make the snapshot "visible" for future opens. Between these events we distinguish:

- **weak meta** — data is in the file but not guaranteed on durable storage;
- **steady meta** — data flushed to disk; the snapshot survives a system crash.

### 10.2. Modes

| Mode                               | Behavior                                                  | Risk                                                                         |
| ---------------------------------- | --------------------------------------------------------- | ---------------------------------------------------------------------------- |
| `MDBX_SYNC_DURABLE` (default)      | Data → flush → meta → flush                               | None: full ACID                                                              |
| `MDBX_NOMETASYNC`                  | Data flushed, meta — deferred                             | Loss of recent commits on a crash                                            |
| `MDBX_SAFE_NOSYNC`                 | Nothing flushed immediately, the previous steady is kept  | Rollback to the last steady; **file growth** (quasi-long-reader effect)      |
| `MDBX_UTTERLY_NOSYNC`              | No flushes, no steady guarantees                          | The database may not survive a crash                                         |
| `MDBX_WRITEMAP`                    | Writing via mmap (+msync)                                 | Combines with the modes above                                                |

**Fragment from [`examples/c++/13-sync-modes.c++`](examples/c++/13-sync-modes.c++)** — changing the sync mode between opens of the same database: full durability (`robust_synchronous`) → `NOMETASYNC` (`half_synchronous_weak_last`):

```cpp
    // Run 1: full durability (sync before the commit returns).
    {
      auto env = example::env_open(path, mdbx::env::operate_parameters().robust_synchronous());
      auto txn = env.start_write();
      auto table = txn.open_map(nullptr);
      txn.insert(table, mdbx::slice("key"), mdbx::slice("v1"));
      txn.commit();
      env.sync_to_disk(true); // explicit flush to disk
      std::cout << "durable: ok\n";
    }

    // Run 2: NOMETASYNC (metadata is not synchronized on every commit).
    {
      auto env = example::env_open(path, mdbx::env::operate_parameters().half_synchronous_weak_last());
      auto txn = env.start_read();
      auto table = txn.open_map(nullptr);
      std::cout << "nometasync: " << txn.get(table, mdbx::slice("key")).as_string() << "\n";
      txn.abort();
    }
```

Full code: [13-sync-modes.c++](examples/c++/13-sync-modes.c++) · [C version](examples/c/13-sync-modes.c)

### 10.3. Which mode when

- **Default** — maximum reliability (ACID). For databases where losing a commit is unacceptable.
- **`MDBX_NOMETASYNC`** — high-load writing where the "tail" of the latest commits may be lost.
- **`MDBX_SAFE_NOSYNC`** — writing can speed up several times (up to 10×), but: the file grows (pages
  newer than steady are not reused until a new steady-point), on a crash — rollback to the last steady.
- **`MDBX_UTTERLY_NOSYNC`** — only for non-critical data/caches.
- **`MDBX_WRITEMAP`** — for large transactions and fine-grained write tuning; combines with the flags
  above (e.g., `SAFE_NOSYNC | WRITEMAP`).

### 10.4. Auto-synchronization

`MDBX_SAFE_NOSYNC` without growth management is a source of uncontrolled file growth. Manage it:

```c
mdbx_env_set_syncbytes(env, 512ULL * 1024 * 1024); /* flush after 512 MB of writes */
mdbx_env_set_syncperiod(env, 3 << 16);             /* 3 seconds in 16.16 format: 3<<16 = 3·65536 units (raw value 3 ≈ 46 µs) */
```

And manually from a separate thread: `mdbx_env_sync_ex(env, force, nonblock)` — with
`nonblock=true` the call returns immediately (asynchronously), completion is checked with the cheap
`mdbx_env_sync_poll(env)`.

### 10.5. Platform-specific notes

- **macOS**: by default `fcntl(F_FULLFSYNC)` — maximum durability but slow;
  `MDBX_APPLE_SPEED_INSTEADOF_DURABILITY` switches the priority to speed.
- **Linux**: `boot_id` is used to control the rollback of weak metas (important in LXC).
- **Windows**: `LockFileEx` file locks are slower than named mutexes (affects small
  transactions); `MDBX_WRITEMAP` helps. Write-through is always used; `MDBX_NOMETASYNC`
  switches to "lazy + flush" (the writethrough threshold is ignored).

### 10.6. Example: switching the mode

**Fragment (C, illustration):**

```c
MDBX_env *env;
mdbx_env_create(&env);
mdbx_env_set_geometry(env, -1, -1, 8LL * 1024 * 1024 * 1024, -1, -1, -1);
mdbx_env_set_flags(env, MDBX_SAFE_NOSYNC, 1);   /* enable */
mdbx_env_set_syncbytes(env, 256ULL * 1024 * 1024);
mdbx_env_open(env, "./safe.mdbx", MDBX_NOSUBDIR, 0664);
```

> **Examples for this chapter:** [`examples/c++/13-sync-modes.c++`](examples/c++/13-sync-modes.c++) · [C version](examples/c/13-sync-modes.c);
> end-to-end project: [`config-store-10.c++`](examples/config-store/config-store-10.c++).

### 10.7. Summary of chapter 10

- Durability modes differ in what gets flushed: data, meta, nothing.
- `SAFE_NOSYNC` — speed at the cost of file growth and rollback to steady on a crash.
- Auto-synchronization (`syncbytes`/`syncperiod`) is mandatory for `SAFE_NOSYNC`.
- Platform differences (F_FULLFSYNC, boot_id, LockFileEx) affect the choice.

### 10.8. Exercises

1. Compare commit speed in `DURABLE` vs `SAFE_NOSYNC` on 10,000 small transactions.
2. Observe file growth in `SAFE_NOSYNC` without auto-sync — and after configuring `syncbytes`.
3. Explain why `UTTERLY_NOSYNC` is "maximum risk": what exactly may not survive a crash.

### 10.9. Chapter 10 checklist

- [ ] I understand that a strict commit is "data to disk + meta to disk" and distinguish weak/steady meta;
- [ ] I know the risks of each mode: `MDBX_SYNC_DURABLE`, `MDBX_NOMETASYNC`, `MDBX_SAFE_NOSYNC`, `MDBX_UTTERLY_NOSYNC`, and combinations with `MDBX_WRITEMAP`;
- [ ] for `MDBX_SAFE_NOSYNC` I configure auto-synchronization (`syncbytes`/`syncperiod`) to keep file growth under control;
- [ ] I can switch modes and call `mdbx_env_sync_ex()` manually;
- [ ] I remember the platform nuances: `F_FULLFSYNC` on macOS, `boot_id` on Linux, `LockFileEx` on Windows.

### 10.10. What's next

Durability modes set the rules for writing; the next chapter is about working with the database
from several threads. Chapter 11 covers sticky threads, the `MDBX_NOSTICKYTHREADS` flag and its
pitfalls, TLS reader slots, `fork()`, transaction cloning, and parking of long-lived readers.
A configurator with a thread pool or coroutines cannot do without this knowledge.

---

## Chapter 11. Multithreading

### 11.1. Thread model: sticky threads by default

By default a transaction "sticks" to the thread that created it. Other threads cannot use this
object — you get `MDBX_THREAD_MISMATCH`. Readers (different transactions in different
threads) work in parallel without locks.

**Fragment from [`examples/c++/14-multithreading.c++`](examples/c++/14-multithreading.c++)** — a writer commits new records, while readers on clones of one snapshot (`base.clone()`) keep seeing a stable picture:

```cpp
    std::atomic<bool> writer_done{false};
    std::thread writer([&] {
      for (int batch = 0; batch < 2; ++batch) {
        auto txn = env.start_write();
        auto table = txn.open_map(nullptr);
        for (int i = 0; i < 10; ++i) {
          const int k = 10 + batch * 10 + i;
          txn.insert(table, mdbx::slice(std::to_string(k)), mdbx::slice("w"));
        }
        txn.commit();
        std::this_thread::yield();
      }
      writer_done = true;
    });

    // Two readers: each works with its own clone of the base snapshot.
    std::atomic<size_t> snapshot_count[2];
    std::atomic<size_t> fresh_count[2];
    std::vector<std::thread> readers;
    for (int r = 0; r < 2; ++r) {
      readers.emplace_back([&, r] {
        auto clone = base.clone(); // a snapshot clone for this thread
        auto table = clone.open_map(nullptr);
        snapshot_count[r] = clone.get_map_stat(table).ms_entries;
        clone.abort();
```

Full code: [14-multithreading.c++](examples/c++/14-multithreading.c++)

### 11.2. MDBX_NOSTICKYTHREADS

The `MDBX_NOSTICKYTHREADS` flag allows transferring a transaction between threads — needed for
thread pools, coroutines, async runtimes (tokio and the like), where an operation may continue in
another thread.

> **Warning:** with `NOSTICKYTHREADS`, functions requiring the environment write lock
> (`mdbx_env_set_option`, `set_flags`, `set_geometry`, `sync`, `stat`, `defrag`, `close`),
> may deadlock if a write transaction "moved" to another thread that waits for
> that lock. Serialize such calls (channel/mutex), or keep them in the thread that owns
> the transaction.

### 11.3. Thread registration and TLS

A reader slot is bound to a thread via TLS. On proper thread exit the destructor
cleans up the slot. If a thread exits "abnormally" (or the TLS destructor did not run — known
glibc bugs #21031/#21032), the slot may "leak". For threads without TLS — explicit registration:

```c
mdbx_thread_register(env);
/* ... */
mdbx_thread_unregister(env);
```

Leak diagnostics — `mdbx_reader_check(env, &dead)` (Volume V, chapter 30).

### 11.4. fork()

After `fork()` the child **does not inherit** mmap and record locks. Using the environment in
the child process is possible only after:

```c
mdbx_env_resurrect_after_fork(env);
```

Called once in the child, not in the parent.

### 11.5. Cloning transactions

`mdbx_txn_clone()` duplicates a read-only transaction — several handlers on one snapshot
without rescanning the RLT (the RLT — reader lock table — is libmdbx's internal registry of
readers; detailed in Volume III, ch. 14) — useful for parallel processing.

### 11.6. Parking and eviction

A long-lived reader "pins" the **detent** (the oldest active snapshot; details — Volume III,
chapter 14) and prevents page recycling. Solutions:

```c
mdbx_txn_park(txn);   /* release the reader slot while keeping the handle */
mdbx_txn_unpark(txn); /* resume (when evicted — with restart_if_ousted=true) */
```

If the writer lacks space, parked readers are **ousted**: the slot
moves from `PARKED` to `OUSTED`; on the next access the reader either restarts on a fresh
snapshot, or gets `MDBX_OUSTED`.

> **Warning:** after parking the snapshot is not held — dereferencing pointers obtained before
> parking is forbidden (pages may be reused).

### 11.7. Several environments; MDBX_EXCLUSIVE

- You cannot open the same database twice in one process (protection against races; legacy mode —
  `MDBX_DBG_LEGACY_MULTIOPEN`).
- Several different databases (different files) in one process — allowed.
- `MDBX_EXCLUSIVE` — exclusive open (only this process).

> **Examples for this chapter:** [`examples/c++/14-multithreading.c++`](examples/c++/14-multithreading.c++);
> [`examples/c++/15-fork-resurrect.c++`](examples/c++/15-fork-resurrect.c++);
> [`examples/c++/16-parking.c++`](examples/c++/16-parking.c++);
> end-to-end project: [`config-store-11.c++`](examples/config-store/config-store-11.c++).

### 11.8. Summary of chapter 11

- Sticky threads by default; `NOSTICKYTHREADS` for pools/coroutines, but with a deadlock risk in
  write functions.
- TLS destructors clean reader slots; on abnormal exits — `reader_check`/registration.
- `resurrect_after_fork` is mandatory in the child.
- Parking/eviction — the solution for long reads.

### 11.9. Exercises

1. Write an example with two threads: one writes, the other reads. Verify the reader is not blocked.
2. Reproduce the scenario: a write transaction started in thread A, thread B calls `env_sync` with
   `NOSTICKYTHREADS` — capture the deadlock.
3. Think of when `txn_clone` saves resources compared to several `txn_begin`.

### 11.10. Chapter 11 checklist

- [ ] I understand that a transaction is bound to its thread by default and recognize `MDBX_THREAD_MISMATCH`;
- [ ] I know when `MDBX_NOSTICKYTHREADS` is justified and which write functions may deadlock with it;
- [ ] I understand how TLS reader slots are cleaned up and how `mdbx_thread_register()`/`mdbx_thread_unregister()` help;
- [ ] after `fork()` I call `mdbx_env_resurrect_after_fork()` in the child, not in the parent;
- [ ] I can clone read-only transactions (`mdbx_txn_clone()`) for parallel processing of one snapshot;
- [ ] I use `mdbx_txn_park()`/`mdbx_txn_unpark()` for long-lived readers and remember that a parked snapshot is not held.

### 11.11. What's next

One practical skill remains: reacting to errors. Chapter 12 systematizes the return codes: expected
states (`MDBX_KEYEXIST`, `MDBX_NOTFOUND`), space problems (`MDBX_MAP_FULL` vs `MDBX_TXN_FULL`),
corruption and discipline-violation errors, and retry loops for concurrent scenarios. After it, the
configurator will withstand both crashes and contention.

---

## Chapter 12. Error handling

### 12.1. Return codes

- `MDBX_SUCCESS` (0) — success.
- Negative `MDBX_*` — errors (`MDBX_NOTFOUND`, `MDBX_KEYEXIST`, `MDBX_BAD_DBI`, ...).
- Special results: `MDBX_RESULT_TRUE` (operation performed), `MDBX_RESULT_FALSE` (no
  data/not required).
- System codes (errno-like) may come from the OS — they are negative and reported as-is.

### 12.2. mdbx_strerror_r vs mdbx_strerror

`mdbx_strerror(rc)` uses a shared buffer — **not thread-safe**. In multithreaded code
use:

```c
char buf[128];
mdbx_strerror_r(rc, buf, sizeof(buf));
```

### 12.3. MDBX_MAP_FULL: what to do

`MDBX_MAP_FULL` — the database hit `upper`, and there is nothing to reuse (the GC is empty/frozen). The sequence:

1. **Abort the current write transaction** (continuing after a geometry change → `MDBX_BAD_TXN`).
2. Check long readers (`mdbx_stat -r` retained; `mdbx_env_info_ex`).
3. Set/raise `upper` **before creating/at open**.
4. Install an HSR callback (HSR — Handle-Slow-Readers, the mechanism for evicting stuck
   readers; Volume V, chapter 29).

### 12.4. MAP_FULL vs TXN_FULL

- `MDBX_MAP_FULL` — the file space is exhausted (geometry).
- `MDBX_TXN_FULL` — the internal limit of the transaction's dirty/retired pages is exhausted. The cure —
  commit earlier, split transactions, check `mdbx_txn_info()`.

### 12.5. Expected codes

- `MDBX_KEYEXIST` — the key already exists (with `NOOVERWRITE`).
- `MDBX_NOTFOUND` — there is no key/value.
  These are normal states, not errors: handle them with branching, not panic.

### 12.6. Corruption and recovery errors

- `MDBX_BAD_TXN`, `MDBX_BAD_DBI` — an invalid handle (discipline violation/closed transaction).
- `MDBX_EBADSIGN` — corrupted signature.
- `MDBX_WANNA_RECOVERY` — the database requires recovery at read-only open; open read-write
  or use `mdbx_env_open_for_recovery()`.
- `MDBX_MVCC_RETARDED` — a reader older than the actual snapshot.
- `MDBX_LAGGARD_READER` — a reader lagged behind (see HSR).

### 12.7. Discipline violations

- `MDBX_THREAD_MISMATCH` — the transaction is used from a foreign thread.
- `MDBX_TXN_OVERLAPPING` — overlapping read/write transactions in one thread.
- `MDBX_BUSY` — an ownership conflict.
- `MDBX_BAD_RSLOT` — an invalid reader slot.
  These are signals of architectural errors, not "random glitches".

### 12.8. Retry loop strategies

For concurrent scenarios ("compare-and-replace"):

**Fragment (C, illustration):**

```c
for (;;) {
    mdbx_txn_begin(env, NULL, MDBX_TXN_READWRITE, &txn);
    /* read the condition */
    rc = mdbx_get(txn, dbi, &key, &val);
    if (rc) { mdbx_txn_abort(txn); return rc; }
if (!condition_met(val)) { mdbx_txn_abort(txn); return MDBX_RESULT_FALSE; }
    /* write the new value */
    rc = mdbx_put(txn, dbi, &key, &newval, 0);
    if (rc) { mdbx_txn_abort(txn); return rc; }
    rc = mdbx_txn_commit(txn);
    if (rc == MDBX_MAP_FULL) { /* raise upper/wait for readers */ continue; }
    return rc;
}
```

**Fragment from [`examples/c++/17-error-handling.c++`](examples/c++/17-error-handling.c++)** — a retry loop on environment "busyness" (`MDBX_BUSY` under process contention / `EAGAIN` inside the process): opening in exclusive mode while the database is already open is repeated a bounded number of times:

```cpp
    // Retry strategy: attempting to open the environment in exclusive mode while
    // it is already open by the current process yields "busyness" (MDBX_BUSY under
    // process contention or the system EAGAIN inside the process).
    // Such a failure is caught and handled with a bounded retry.
    const int max_attempts = 3;
    int attempt = 0;
    for (; attempt < max_attempts; ++attempt) {
      try {
        mdbx::env_managed busy(path, mdbx::env::operate_parameters().exclusive());
        std::cerr << "FAIL: exclusive open unexpectedly succeeded\n";
        return EXIT_FAILURE;
      } catch (const std::exception &ex) {
        std::cout << "busy attempt " << (attempt + 1) << ": failed (" << ex.what() << ")\n";
      }
    }
    std::cout << "busy: persists after " << attempt << " attempts (expected while env is open)\n";
```

Full code: [17-error-handling.c++](examples/c++/17-error-handling.c++) · [C version](examples/c/17-error-handling.c)

> **Examples for this chapter:** [`examples/c++/17-error-handling.c++`](examples/c++/17-error-handling.c++) · [C version](examples/c/17-error-handling.c);
> end-to-end project: [`config-store-12.c++`](examples/config-store/config-store-12.c++).

### 12.9. Summary of chapter 12

- Success = 0; errors are negative; `RESULT_TRUE/FALSE` — special results.
- `mdbx_strerror_r` — the thread-safe variant.
- `MAP_FULL` is cured with geometry+HSR; `TXN_FULL` — with early commits.
- `KEYEXIST`/`NOTFOUND` — expected states.
- Discipline codes (`THREAD_MISMATCH` and others) indicate architectural errors.

### 12.10. Exercises

1. Write a `MDBX_KEYEXIST` handler in the end-to-end project: on conflict — read and decide.
2. Describe how `MAP_FULL` differs from `TXN_FULL`, with examples.
3. Make a retry loop around `cfg_set` with a "read-check-write" strategy.

### 12.11. Chapter 12 checklist

- [ ] I read return codes correctly: 0 — success, negative — errors, `MDBX_RESULT_TRUE`/`MDBX_RESULT_FALSE` — special results;
- [ ] in multithreaded code I use `mdbx_strerror_r()` rather than `mdbx_strerror()`;
- [ ] I know the sequence of actions for `MDBX_MAP_FULL` (abort, check readers, raise `upper` before opening, HSR) and can tell it apart from `MDBX_TXN_FULL`;
- [ ] I handle `MDBX_KEYEXIST` and `MDBX_NOTFOUND` with branching, not panic;
- [ ] I treat discipline codes (`MDBX_THREAD_MISMATCH`, `MDBX_TXN_OVERLAPPING`) as signals of architectural mistakes;
- [ ] I can write a "read-check-write" retry loop for concurrent scenarios.

### 12.12. What's next

Volume II provided the full practical toolkit, but many of its rules ("close read transactions",
"geometry before open", "`SAFE_NOSYNC` grows the file") looked like demands without explanations.
Volume III opens libmdbx's internal mechanisms: B+tree and mmap, MVCC, the commit pipeline, the
free-page GC, geometry, and crash recovery. With them, the rules turn into an understanding of
"why".

---

## Volume summary

Volume II gave you a practical toolkit: cursors, DUPSORT and indexes, environment configuration,
durability modes, multithreading, and error handling. The end-to-end configurator project is now a
full-fledged application with indexes.

**What's next:** Volume III — internal mechanisms: B+tree and mmap, MVCC, the commit pipeline, GC,
geometry, nested transactions, the lock file, durability and recovery.