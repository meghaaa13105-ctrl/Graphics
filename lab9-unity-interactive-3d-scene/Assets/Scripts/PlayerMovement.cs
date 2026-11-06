
using UnityEngine;
using UnityEngine.InputSystem;

public class PlayerMovement : MonoBehaviour
{
    public float speed = 5f;

    void Update()
    {
        if (Keyboard.current == null)
            return;

        Vector3 direction = Vector3.zero;

        if (Keyboard.current.wKey.isPressed ||
            Keyboard.current.upArrowKey.isPressed)
            direction.z += 1f;

        if (Keyboard.current.sKey.isPressed ||
            Keyboard.current.downArrowKey.isPressed)
            direction.z -= 1f;

        if (Keyboard.current.aKey.isPressed ||
            Keyboard.current.leftArrowKey.isPressed)
            direction.x -= 1f;

        if (Keyboard.current.dKey.isPressed ||
            Keyboard.current.rightArrowKey.isPressed)
            direction.x += 1f;

        if (direction.sqrMagnitude > 0)
        {
            transform.Translate(
                direction.normalized * speed * Time.deltaTime,
                Space.World
            );
        }
    }
}
