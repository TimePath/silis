# Style guide

```
           +------------+------------+------------+--------------+------------+------------+-----------+
           | directory  | file       | macro      | namespace    | enum       | struct     | code      |
+----------+------------+------------+------------+--------------+------------+------------+-----------+
| scope    | kebab-case | kebab-case |            | flatcase     | PascalCase | PascalCase |           |
+----------+------------+------------+------------+--------------+------------+------------+-----------+
| function |            |            | MACRO_CASE | snake_case   |            | camelCase  |           |
+----------+------------+------------+------------+--------------+------------+------------+-----------+
| constant |            |            | MACRO_CASE | MACRO_CASE   | PascalCase | MACRO_CASE |           |
+----------+------------+------------+------------+--------------+------------+------------+-----------+
| variable |            |            |            | g_snake_case |            | camelCase_ | camelCase |
+----------+------------+------------+------------+--------------+------------+------------+-----------+
```

## Class order

 - type aliases
 - friends
 - static members
 - members
 - usings
 - destructor
 - constructors
    - no-arg constructor
    - static constructors
    - copy constructor
      - or copy method
    - move constructor (always implicit)
      - use `exchange` or `move`
      - move assign operator
    - implicit conversion constructors
    - other custom constructors
 - accessors
 - explicit non-operator conversions
 - all other public members
   - order: methods, operator conversions
     - bool conversion: always explicit
   - order: const, mutable
 - all other private members
