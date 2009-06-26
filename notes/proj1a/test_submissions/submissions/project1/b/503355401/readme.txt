I tried to optimize performance by specifying INDEX attributes where I
noticed they were being referenced often.

The constraints I chose to enforce were:

Primary key constraints:
  - Every movie has a unique identification number.
  - Every actor has a unique identification number.
  - Every director has a unique identification number.

Referential integrity constraints:
  - Every genre references a valid movie.
  - Every movie/director references a valid movie.
  - Every movie/director references a valid director.
  - Every movie/actor references a valid movie.
  - Every movie/actor references a valid actor.
  - Every review references a valid movie.

CHECK constraints:
  - Every actor's dod comes after their dob or is NULL.
  - Every director's dod comes after their dob or is NULL.
  - Every rating must be >= 0.

