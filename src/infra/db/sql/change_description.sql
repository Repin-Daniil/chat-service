UPDATE chat.channels
SET description = $2
WHERE channel_id = $1;
