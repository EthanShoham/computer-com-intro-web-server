# Web Server API Documentation

## Supported HTTP Methods

The server supports the following HTTP methods:

- `OPTIONS`
- `HEAD`
- `TRACE`
- `GET`
- `POST`
- `PUT`
- `DELETE`

## API Endpoints

### Root

- **GET /**  
  Returns a Lorem Ipsum page
  Optional query parameter:
  - `lang`: Specifies the language of the response. Accepted values are `he`, `en`, or `fr`.

### Students

- **GET /students**  
  Retrieves a list of all saved students.

- **GET /students?id={id}**  
  Retrieves the student with the specified `id`.  
  - If the student does not exist, returns a `404 Not Found` status.

- **POST /students**  
  Creates a new student.  
  - Request body must be a compact JSON object (no whitespace) with the following structure:
    ```json
    {"id":123456789,"fname":"FirstName","lname":"LastName","grade":100}
    ```
    - `id`: 9-digit student ID number.
    - `fname`: First name as a string.
    - `lname`: Last name as a string.
    - `grade`: Grade as an integer (up to 3 digits).

- **PUT /students?id={id}**  
  Updates an existing student or creates a new one with the specified `id`.  
  - Request body must be a compact JSON object (no whitespace) with the following structure:
    ```json
    {"fname":"FirstName","lname":"LastName","grade":100}
    ```
    - `fname`: First name as a string.
    - `lname`: Last name as a string.
    - `grade`: Grade as an integer (up to 3 digits).

- **DELETE /students?id={id}**  
  Deletes the student with the specified `id`.  
  - If the student does not exist, returns a `404 Not Found` status.

