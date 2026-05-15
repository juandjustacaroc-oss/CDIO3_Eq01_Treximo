import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:go_router/go_router.dart';

import 'core/theme/app_theme.dart';
import 'data/database/app_database.dart';
import 'data/repositories/drift_repositories.dart';
import 'data/datasources/http_datasource.dart';
import 'data/datasources/mock_datasource.dart';
import 'domain/entities/entities.dart';
import 'domain/repositories/repositories.dart';
import 'presentation/blocs/session_bloc.dart';
import 'presentation/blocs/live_session_bloc.dart';
import 'presentation/blocs/athlete_bloc.dart';
import 'presentation/screens/screens_a.dart';
import 'presentation/screens/screens_b.dart';
import 'presentation/screens/screens_c.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  final db = AppDatabase();
  final sessionRepo = DriftSessionRepository(db);
  final athleteRepo = DriftAthleteRepository(db);

  // Seed mock athletes if db is empty
  final athletes = await athleteRepo.getAll();
  if (athletes.isEmpty) {
    for (var a in mockAthletes) {
      await athleteRepo.save(a);
    }
    final mockSessions = generateMockSessions();
    for (var s in mockSessions) {
      await sessionRepo.save(s);
    }
  }

  // Por defecto arrancamos con Mock. Cambia a HttpSportDataSource() para Real!
  final dataSource = MockSportDataSource();
  // final dataSource = HttpSportDataSource(port: 8080);

  runApp(MyApp(
    sessionRepo: sessionRepo,
    athleteRepo: athleteRepo,
    dataSource: dataSource,
  ));
}

class MyApp extends StatelessWidget {
  final SessionRepository sessionRepo;
  final AthleteRepository athleteRepo;
  final SportDataSource dataSource;

  MyApp({
    super.key,
    required this.sessionRepo,
    required this.athleteRepo,
    required this.dataSource,
  });

  late final GoRouter router = GoRouter(
    routes: [
      GoRoute(
        path: '/',
        builder: (context, state) => const HomeScreen(),
      ),
      GoRoute(
        path: '/athletes',
        builder: (context, state) => const AthletesScreen(),
      ),
      GoRoute(
        path: '/history',
        builder: (context, state) => const HistoryScreen(),
      ),
      GoRoute(
        path: '/new-session',
        builder: (context, state) => const NewSessionScreen(),
      ),
      GoRoute(
        path: '/session/:id',
        builder: (context, state) {
          final id = state.pathParameters['id']!;
          return SessionDetailScreen(
            sessionId: id,
          );
        },
      ),
      GoRoute(
        path: '/athlete/:id',
        builder: (context, state) {
          final id = int.parse(state.pathParameters['id']!);
          return AthleteDetailScreen(
            athleteId: id,
          );
        },
      ),
      GoRoute(
        path: '/settings',
        builder: (context, state) => const SettingsScreen(),
      ),
      GoRoute(
        path: '/connection',
        builder: (context, state) => const ConnectionScreen(),
      ),
      GoRoute(
        path: '/about',
        builder: (context, state) => const AboutTreximoPage(),
      ),
      GoRoute(
        path: '/how-it-works',
        builder: (context, state) => const HowItWorksPage(),
      ),
      GoRoute(
        path: '/manual',
        builder: (context, state) => const UserManualPage(),
      ),
    ],
  );

  @override
  Widget build(BuildContext context) {
    return MultiBlocProvider(
      providers: [
        BlocProvider(
          create: (_) => AthleteBloc(athleteRepo)..add(LoadAthletes()),
        ),
        BlocProvider(
          create: (_) => SessionBloc(sessionRepo)..add(const LoadSessions()),
        ),
        BlocProvider(
          create: (_) => LiveSessionBloc(sessionRepo, dataSource),
        ),
      ],
      child: MaterialApp.router(
        title: 'Sprint Timer',
        debugShowCheckedModeBanner: false,
        theme: AppTheme.dark,
        routerConfig: router,
      ),
    );
  }
}
