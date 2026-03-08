SELECT role
FROM chat.channel_members
WHERE channel_id = $1 AND user_id = $2