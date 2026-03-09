SELECT title, description
FROM chat.channels
WHERE channel_id = $1;