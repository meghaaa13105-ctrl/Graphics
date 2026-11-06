
using UnityEngine;

public class ClickToChangeColor : MonoBehaviour
{
    private Renderer objectRenderer;
    private Color originalColor;
    private bool isChanged = false;

    void Start()
    {
        objectRenderer = GetComponent<Renderer>();
        originalColor = objectRenderer.material.color;
    }

    void OnMouseDown()
    {
        isChanged = !isChanged;

        if (isChanged)
        {
            objectRenderer.material.color = Color.yellow;
        }
        else
        {
            objectRenderer.material.color = originalColor;
        }
    }
}
