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