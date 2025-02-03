#if (__cplusplus < 201703L)
#error "This library requires C++17 / Espressif32 Arduino 3.x"
#else

const char* webPage = R"webPage(<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>%PAGE_TITLE%</title>
    <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="bg-gray-100 flex items-center justify-center min-h-screen">
    <div class="bg-white p-6 rounded-lg shadow-lg w-full max-w-md">
        <h1 class="text-xl font-bold mb-4 text-center">%PAGE_TITLE%</h1>
        <form id="dynamicForm" class="space-y-4">
            <!-- Dynamische Felder -->
        </form>
        <!-- <button id="submitButton" class="w-full bg-blue-500 text-white py-2 rounded-lg hover:bg-blue-600 transition">Daten senden</button> -->
        <button id="submitButton" class="w-full bg-blue-500 text-white py-2 rounded-lg hover:bg-blue-600 transition mt-4">%BUTTON_TEXT%</button>
    </div>

    <script>
        async function fetchProperties() {
            try {
                const response = await fetch('%BASE_URL%/jsonSchema');
                if (!response.ok) throw new Error('Fehler beim Abrufen der JSON-Schema-Daten');
                const data = await response.json();
                if (!data || !data.properties) throw new Error('Fehlende "properties" Struktur');
                return data.properties;
            } catch (error) {
                console.error('Fehler:', error);
                alert('Fehler beim Laden der Schema-Daten');
            }
        }

        async function fetchData() {
            try {
                const response = await fetch('%BASE_URL%');
                if (!response.ok) throw new Error('Fehler beim Abrufen der API-Daten');
                return await response.json();
            } catch (error) {
                console.error('Fehler:', error);
                alert('Fehler beim Laden der Daten');
            }
        }

        async function createForm() {
            const properties = await fetchProperties();
            if (!properties) return;
            const data = await fetchData() || {};
            const form = document.getElementById('dynamicForm');
            form.innerHTML = '';

            for (const [key, value] of Object.entries(properties)) {
                const wrapper = document.createElement('div');
                wrapper.className = 'flex flex-col';

                const label = document.createElement('label');
                label.setAttribute('for', key);
                label.className = 'font-medium text-gray-700';
                label.innerText = key.charAt(0).toUpperCase() + key.slice(1);

                const input = document.createElement('input');
                input.setAttribute('id', key);
                input.setAttribute('name', key);
                input.className = 'mt-1 p-2 border rounded-lg focus:ring focus:ring-blue-300';

                if (value.type === 'string') {
                    input.setAttribute('type', 'text');
                    input.value = data[key] || '';
                } else if (value.type === 'number') {
                    input.setAttribute('type', 'number');
                    input.value = data[key] || '';
                } else if (value.type === 'boolean') {
                    input.setAttribute('type', 'checkbox');
                    input.className = 'mt-1';
                    input.checked = data[key] || false;
                }

                wrapper.appendChild(label);
                wrapper.appendChild(input);
                form.appendChild(wrapper);
            }
        }

        async function sendData() {
            const jsonData = {};
            document.querySelectorAll('#dynamicForm input').forEach(input => {
                jsonData[input.name] = input.type === 'checkbox' ? input.checked : input.value;
            });

            const response = await fetch('%BASE_URL%', {
                method: 'PATCH',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(jsonData)
            });

            if (!response.ok) alert('Fehler beim Senden der Daten!');
        }

        document.addEventListener('DOMContentLoaded', () => {
            createForm();
            document.getElementById('submitButton').addEventListener('click', sendData);
        });
    </script>
</body>
</html>
)webPage";


#endif