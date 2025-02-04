const char* webPage = R"webPage(<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>%PAGE_TITLE%</title>
    <script src="https://cdn.tailwindcss.com"></script>
</head>
<body class="bg-gray-100 flex items-center justify-center min-h-screen">
    <div class="bg-white p-6 rounded-lg shadow-lg w-full max-w-md relative">
        <h1 class="text-xl font-bold mb-4 text-center">%PAGE_TITLE%</h1>
        <div id="spinner" class="flex justify-center items-center my-4">
            <svg class="animate-spin h-8 w-8 text-blue-500" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
                <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
                <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0a12 12 0 00-12 12h4z"></path>
            </svg>
        </div>
        <form id="dynamicForm" class="space-y-4 hidden"></form>
        <button id="submitButton" class="w-full bg-blue-500 text-white py-2 rounded-lg hover:bg-blue-600 transition mt-4 hidden">%BUTTON_TEXT%</button>
    </div>
    <script>
        async function fetchProperties() {
            try {
                const response = await fetch('%JSON_SCHEMA_ROUTE%');
                if (!response.ok) throw new Error('Error fetching JSON schema data');
                const data = await response.json();
                if (!data || !data.properties) throw new Error('Missing "properties" structure');
                return data.properties;
            } catch (error) {
                console.error('Error:', error);
                alert('Error loading schema data');
            }
        }

        async function fetchData() {
            try {
                const response = await fetch('%API_ROUTE%');
                if (!response.ok) throw new Error('Error fetching API data');
                return await response.json();
            } catch (error) {
                console.error('Error:', error);
                alert('Error loading data');
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
            document.getElementById('spinner').classList.add('hidden');
            form.classList.remove('hidden');
            document.getElementById('submitButton').classList.remove('hidden');
        }

        async function sendData() {
            const jsonData = {};
            document.querySelectorAll('#dynamicForm input').forEach(input => {
                jsonData[input.name] = input.type === 'checkbox' ? input.checked : input.value;
            });
            const response = await fetch('%API_ROUTE%', {
                method: 'PATCH',
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