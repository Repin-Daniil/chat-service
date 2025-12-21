UPDATE chat.users
SET 
    username = COALESCE($2, username),
    display_name = COALESCE($3, display_name),
    biography = COALESCE($4, biography),
    password_hash = COALESCE($5, password_hash),
    salt = COALESCE($6, salt)
WHERE username = $1
RETURNING username;