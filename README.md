# Web Server
## Supported Methods:
OPTIONS, HEAD, TRACE, GET, POST, PUT, DELETE

## Resources
- GET / : with optional query parameter lang (he, en, fr).
- GET /students : retrive a list of current saved students.
- GET /students?id={id} : retrive a student by id, if doesn't exists returns 404.
- POST /students : create a new student from compact json body with no white spaces ({"id":(9 digits id number),"fname":"(first name here as string)","lname":"(last name here as string)","grade":(up to 3 digit number for grade)}).
- PUT /students?id={id} : edit or create the student with the give id from compact json body with no white spaces ({"fname":"(first name here as string)","lname":"(last name here as string)","grade":(up to 3 digit number for grade)}).
- DELETE /student?id={id} : deletes the given student if doesn't exists returns 404.
