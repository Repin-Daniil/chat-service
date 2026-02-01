WITH params AS (
    SELECT 
        $1::text AS input_chat_id,
        $2 AS u1,
        $3 AS u2
),
existing AS (
    SELECT chat_id 
    FROM chat.private_chats p
    JOIN params par ON p.user_id_1 = par.u1 AND p.user_id_2 = par.u2
),
insert_chat AS (
    INSERT INTO chat.chats (chat_id, type)
    SELECT input_chat_id, 'PRIVATE'
    FROM params
    WHERE NOT EXISTS (SELECT 1 FROM existing)
    RETURNING chat_id
),
insert_private AS (
    INSERT INTO chat.private_chats (chat_id, user_id_1, user_id_2)
    SELECT ic.chat_id, p.u1, p.u2
    FROM insert_chat ic
    CROSS JOIN params p
    RETURNING chat_id
)

SELECT chat_id, true as is_new FROM insert_private
UNION ALL
SELECT chat_id, false as is_new FROM existing;