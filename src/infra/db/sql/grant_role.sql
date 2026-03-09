UPDATE chat.channel_members
SET role = $3
WHERE channel_id = $1 AND user_id = $2;
