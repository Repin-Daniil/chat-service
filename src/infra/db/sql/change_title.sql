UPDATE chat.channels
SET title = $2
WHERE channel_id = $1;
