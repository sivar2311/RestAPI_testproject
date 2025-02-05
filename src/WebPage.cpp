#if (__cplusplus < 201703L)
#error "This library requires C++17 / Espressif32 Arduino 3.x"
#else

const char* webPage = R"webPage(
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>%PAGE_TITLE%</title>
    <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="bg-gray-100 flex items-center justify-center min-h-screen">
    <div class="bg-white p-6 rounded-lg shadow-lg w-full max-w-md relative">
        <h1 class="text-xl font-bold mb-4 text-center">%PAGE_TITLE%</h1>
        <form id="dynamicForm" class="space-y-4 hidden"></form>
        <button id="submitButton" class="w-full bg-blue-500 text-white py-2 rounded-lg hover:bg-blue-600 transition mt-4 hidden">%BUTTON_TEXT%</button>
    </div>
    <script>
        async function fetchSchema() {
            try {
                const response = await fetch('%FORM_ROUTE%');
                if (!response.ok) throw new Error('Error fetching JSON schema data');
                return await response.json();
            } catch (error) {
                console.error('Error:', error);
                alert('Error loading schema data');
            }
        }

        async function createForm() {
            const schema = await fetchSchema();
            if (!schema) return;

            const form = document.getElementById('dynamicForm');
            form.innerHTML = ''; // Clear any existing form fields

            for (const [key, value] of Object.entries(schema)) {
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
                    input.setAttribute('type', value.password ? 'password' : 'text');
                    input.value = value.value || '';
                } else if (value.type === 'number') {
                    input.setAttribute('type', 'number');
                    input.value = value.value || '';
                    if (value.min !== undefined) input.setAttribute('min', value.min);
                    if (value.max !== undefined) input.setAttribute('max', value.max);
                } else if (value.type === 'boolean') {
                    input.setAttribute('type', 'checkbox');
                    input.className = 'mt-1';
                    input.checked = value.value || false;
                }

                wrapper.appendChild(label);
                wrapper.appendChild(input);
                form.appendChild(wrapper);
            }

            form.classList.remove('hidden');
            document.getElementById('submitButton').classList.remove('hidden');
        }

        async function sendData() {
            const jsonData = {};
            document.querySelectorAll('#dynamicForm input').forEach(input => {
                jsonData[input.name] = input.type === 'checkbox' ? input.checked : input.value;
            });

            const response = await fetch('%FORM_ROUTE%', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(jsonData)
            });

            if (!response.ok) alert('Error sending data!');
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