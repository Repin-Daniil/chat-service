INSERT INTO chat.users(user_id, username, display_name, biography, password_hash, salt)
VALUES($1, $2, $3, $4, $5, $6)